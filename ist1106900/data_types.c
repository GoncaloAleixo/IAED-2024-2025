/**
 * @file data_types.c
 * @author ist1106900 (Goncalo Aleixo)
*/


// header
#include "data_types.h"

// c stdlib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* -------------------------------------------------------------------------- */
/*                                    date                                    */
/* -------------------------------------------------------------------------- */

bool is_leap_year(u32 year) {

    // pela definição, um ano bissexto é divisível por 4, exceto se for
    // divisível por 100, a menos que seja divisível por 400.
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool date_is_valid(Date const* date) {
    
    // checa pelos limites de data, exceto pelo limite superior do dia, que é mais complexo...
    if (date->year < 1 || date->month < 1 || date->month > 12 || date->day < 1)
        return false;

    // lista de últimos dias de cada mês, determinando o último dia de fevereiro com base no número do ano
    int days_in_month[] = {31, (is_leap_year(date->year) ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // checa se o dia é válido para o mês
    if (date->day > days_in_month[date->month - 1])
        return false;

    return true;
}


i8 date_cmp(Date const* lhs, Date const* rhs) {

    // comparação do ano
    if (lhs->year < rhs->year)
        return -1;
    if (lhs->year > rhs->year)
        return 1;

    // comparação do mês
    if (lhs->month < rhs->month)
        return -1;
    if (lhs->month > rhs->month)
        return 1;

    // comparação do dia
    if (lhs->day < rhs->day)
        return -1;
    if (lhs->day > rhs->day)
        return 1;

    return 0;
}


void date_put(Date const* date) {
    printf("%02u-%02u-%u", date->day, date->month, date->year);
}



/* -------------------------------------------------------------------------- */
/*                                   vector                                   */
/* -------------------------------------------------------------------------- */

/* --------------------------- internal functions --------------------------- */

bool vector_realloc(Vector* vec, usize new_size, ErrorState* estate) {

    assert(new_size >= vec->size, "new size is less than current size");

    // realoca o vetor com a nova capacidade
    void* real_data = (void*)((u8*)vec->data - vec->element_size); // endereço do início real do vetor
    if (vec->capacity == 0)
        real_data = NULL;

    void* new_data = realloc(real_data, (new_size + 1) * vec->element_size); // ponteiro para o novo buffer
    if (new_data == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }
    
    vec->data = &((u8*)new_data)[vec->element_size];
    vec->capacity = new_size;
    return true;
}

bool vector_grow(Vector* vec, ErrorState* estate) {

    // dobra a capacidade a cada vez que a capacidade atual é atingida
    // se a capacidade atual for 0(vetor vazio), a nova capacidade será 1
    usize new_capacity = vec->capacity * VECTOR_GROWTH_FACTOR; // nova capacidade
    if (new_capacity == 0)
        new_capacity = 1;

    assert(new_capacity > vec->capacity, "new capacity is not greater than current capacity");
    return vector_realloc(vec, new_capacity, estate);
}


/* ---------------------------- public functions ---------------------------- */

void vector_init(Vector* vec, usize element_size, void(*free_fn)(void*)) {
    vec->free_fn = free_fn;
    vec->element_size = element_size;
    vec->capacity = 0;
    vec->size = 0;
    vec->data = NULL;
}

void vector_free(void* vec) {

    Vector* vector = (Vector*)vec; // ponteiro convertido para o tipo correto

    // itera sobre todos os elementos que restaram no vetor e os libera com a função de liberação recebida
    if (vector->free_fn != NULL) {
        for (usize i = 0; i < vector->size; i++) {
            void* item = vector_at(vector, i); // endereço do item
            vector->free_fn(item);
        }
    }

    // libera o buffer do vetor em sí, após ter destruído todos os elementos
    if (vector->capacity == 0)
        return;

    void* real_data = (void*)((u8*)vector->data - vector->element_size); // endereço do início real do vetor
    free(real_data);
}

usize vector_size(Vector const* vec) {
    return vec->size;
}

void* vector_at(Vector* vec, usize idx) {
    
    if (idx >= vec->size)
        return NULL;
    
    return (void*)((u8*)vec->data + (idx * vec->element_size));
}

bool vector_insert(Vector* vec, usize idx, void* data, ErrorState* estate) {
    
    // verifica se a capacidade do vetor é suficiente para inserir um novo elemento
    // caso não seja, chama a função para aumentar a capacidade do vetor
    if (vec->size == vec->capacity) {
        if (!vector_grow(vec, estate)) {
            vec->free_fn(data);
            return false;
        }
    }
    
    // se o índice for maior que o tamanho do vetor, insere no final
    // se não, insere na posição especificada
    if (idx > vec->size)
        idx = vec->size;
    
    // calcula os endereços de onde o dado será inserido e de onde os dados serão movidos
    // e também o endereço para onde os dados que ficarão após o novo item devem ser movidos
    void* dst = (void*)((u8*)vec->data + (idx * vec->element_size));       // endereço onde o novo dado será inserido
    void* src = (void*)((u8*)vec->data + ((idx + 1) * vec->element_size)); // endereço onde os dados que ficarão após o novo item devem ser movidos
    usize size = (vec->size - idx) * vec->element_size;                    // quantidade de bytes a serem movidos
    
    // executa a movimentação dos dados e atualiza o tamanho do vetor
    memmove(src, dst, size);
    memcpy(dst, data, vec->element_size);
    vec->size += 1;
    
    return true;
}

bool vector_remove(Vector* vec, usize idx, void* out_value_dst, ErrorState* estate) {
    
    // verifica se o índice é válido
    if (idx >= vec->size) {
        internal_error("idx out of bounds");
        return false;
    }
    
    // calcula os endereços de onde o dado será removido e de onde os dados serão movidos
    // e também o endereço para onde os dados que ficarão após o item removido devem ser movidos
    void* dst = (void*)((u8*)vec->data + (idx * vec->element_size));       // endereço onde o dado será removido
    void* src = (void*)((u8*)vec->data + ((idx + 1) * vec->element_size)); // endereço onde os dados que ficarão após o item removido devem ser movidos
    usize size = (vec->size - idx - 1) * vec->element_size;                // quantidade de bytes a serem movidos
    
    // caso 'out_value_dst' seja um endereço válido, copia o valor removido para esse endereço
    // caso contrário, chama a função de liberação do vetor para liberar o valor removido
    if (out_value_dst != NULL)
        memcpy(out_value_dst, dst, vec->element_size);
    else if (vec->free_fn != NULL)
        vec->free_fn(dst);

    // executa a movimentação dos dados para ocupar o espaço vazio e atualiza o tamanho do vetor
    memmove(dst, src, size);
    vec->size -= 1;

    if (vec->capacity > (VECTOR_GROWTH_FACTOR * 2) && (vec->size <= ((vec->capacity / VECTOR_GROWTH_FACTOR) / 2)))
        if (!vector_realloc(vec, (vec->capacity / VECTOR_GROWTH_FACTOR), estate))
            return false;


    return true;
}

usize vector_binary_search(Vector* vec, void* data, bool* out_found, i8(*cmp_fn)(void*, void*, void*), void* cmp_fn_ctx) {

    // retorna o índice onde o dado deve ser inserido, ou o índice do dado se ele já existir
    // a busca é feita de forma binária, ou seja, o vetor deve estar ordenado
    // o vetor deve ser ordenado de acordo com a função de comparação recebida

    if (vec->size == 0) {
        *out_found = false;
        return 0;
    }

    usize left = 0; // índice inicial
    usize right = vec->size - 1; // índice final

    while (left <= right) {
        
        usize mid = left + (right - left) / 2; // índice do item do meio
        void* mid_elem = vector_at(vec, mid); // elemento do meio
        i8 cmp_result = cmp_fn(mid_elem, data, cmp_fn_ctx); // resultado da comparação

        if (cmp_result == 0) {
            *out_found = true;
            return mid;
        }

        if (cmp_result < 0) {
            
            left = mid + 1;
            if (mid == vec->size)
                break;
                
        } else {
        
            if (mid == 0)
                break;
            
            right = mid - 1;
        }

    }

    *out_found = false;
    return left;
}

bool vector_resize(Vector* vec, usize new_size, void* init_data, ErrorState* estate) {

    // verifica se o novo tamanho é maior que o tamanho atual
    if (new_size < vec->size) {
        if (vec->free_fn != NULL) {
            for (usize i = new_size; i < vec->size; i++) {
                void* item = vector_at(vec, i); // endereço do item
                vec->free_fn(item);
            }
        }
    }

    // verifica se a nova capacidade é maior que a capacidade atual
    // caso não seja, chama a função para aumentar a capacidade do vetor
    if (new_size > vec->capacity) {
        if (!vector_realloc(vec, new_size, estate))
            return false;
    }

    if (init_data != NULL) {
        for (usize i = vec->size; i < new_size; i++) {
            void* item = (void*)((u8*)vec->data + (i * vec->element_size)); // endereço do item
            memcpy(item, init_data, vec->element_size);
        }
    }

    // atualiza o tamanho do vetor
    vec->size = new_size;
    return true;
}

void vector_clear(Vector* vec) {
    
    // itera sobre todos os elementos que restaram no vetor e os libera com a função de liberação recebida
    
    if (vec->free_fn != NULL) {
        for (usize i = 0; i < vec->size; i++) {
            void* item = vector_at(vec, i); // endereço do item
            vec->free_fn(item);
        }
    }

    // reseta o tamanho do vetor
    vec->size = 0;
}

void vector_string_free(void* ptr) {
    free(*(char**)ptr);
}



/* -------------------------------------------------------------------------- */
/*                                name_hashmap                                */
/* -------------------------------------------------------------------------- */

/* --------------------------- internal functions --------------------------- */

usize name_hashmap_hash(const char* key) {

    // implementação do algoritmo de hash "Simple WyHash"

    // constantes de multiplicação do algoritmo
    const usize _wyp0 = 0xa0761d6478bd642full;
    const usize _wyp1 = 0xe7037ed1a0b428dbull;
    
    usize hash = _wyp0; // valor do hash, inicializado com a constante de multiplicação
    while (*key) {
        hash ^= (*key) * _wyp1;
        hash *= _wyp0;
        key += 1;
    }

    return hash;
}

usize name_hashmap_calc_bucket(NameHashMap* map, const char* key) {

    usize hash = name_hashmap_hash(key); // hash da chave
    usize bucket = hash % vector_size(&map->buckets_vec); // modulo do hash pelo tamanho do vetor de buckets
    return bucket;
}

bool name_hashmap_find_key_entry_bucket_idx(NameHashMap* map, const char* key, usize bucket_idx, usize* out_idx) {

    // obtém o índice do primeiro elemento do bucket
    usize entry_idx = *(usize*)vector_at(&map->buckets_vec, bucket_idx);

    // se o bucket estiver vazio, retorna false e U32_MAX, indicando que não encontrou a chave, e que não há entradas
    // no bucket, logo não é possível indicar a entrada que ficaria ao lado(antes ou depois) da nova entrada
    if (entry_idx == U32_MAX) {
        *out_idx = U32_MAX;
        return false;
    }

    // percorre o vetor de entradas do bucket até encontrar a chave, chegar ao final do bucket, ou encontrar uma entrada
    // que seja maior que a chave(o que indica que a chave não existe)
    while (entry_idx != U32_MAX) {

        // obtém a referência da entrada
        NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx);

        // verifica se a chave é igual
        int strcmp_res = strcmp(key, entry->key);
        if (strcmp_res == 0) {
            *out_idx = entry_idx;
            return true;
        }

        // se a chave for maior que a atual e a atual for a última do bucket,
        // então retorna false e o índice da entrada atual, que é a última do bucket
        if (strcmp_res > 0) {
            if (entry->after_idx == U32_MAX) {
                *out_idx = entry_idx;
                return false;
            }
            entry_idx = entry->after_idx;
            continue;
        }

        // se a chave for menor, não há mais entradas a serem verificadas
        if (strcmp_res < 0) break;
    }

    // se a chave não foi encontrada, mas o índice de uma entrada maior que a chave
    // foi encontrado, verifica se há uma entrada antes dela, e se sim, retorna o índice dela
    // se não, retorna o índice da entrada atual, que é a primeira maior que a chave
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx);
    if (entry->before_idx != U32_MAX) *out_idx = entry->before_idx;
    else                                *out_idx = entry_idx;
    return false;
}

void name_hashmap_insert_entry_before_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_entry_idx) {

    // obtém a referência das entradas
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx);
    NameHashMapEntry* bucket_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, bucket_entry_idx);
    assert(entry != NULL && bucket_entry != NULL, "entry or bucket_entry is null");

    // se a entrada for a primeira do bucket, insere a nova entrada como a primeira do bucket
    if (bucket_entry->before_idx == U32_MAX) {
        entry->after_idx = bucket_entry_idx;
        bucket_entry->before_idx = entry_idx;
        entry->before_idx = U32_MAX;
        *(usize*)vector_at(&map->buckets_vec, name_hashmap_calc_bucket(map, entry->key)) = entry_idx;
        return;
    }

    // se a entrada atual não for a primeira do bucket, insere a nova entrada antes dela
    NameHashMapEntry* before_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, bucket_entry->before_idx);
    assert(before_entry != NULL, "before entry is null");

    entry->before_idx = bucket_entry->before_idx;
    entry->after_idx = bucket_entry_idx;
    before_entry->after_idx = entry_idx;
    bucket_entry->before_idx = entry_idx;
}

void name_hashmap_insert_entry_after_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_entry_idx) {

    // obtém a referência das entradas
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx);
    NameHashMapEntry* bucket_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, bucket_entry_idx);
    assert(entry != NULL && bucket_entry != NULL, "entry or bucket_entry is null");

    // se a entrada for a última do bucket, insere a nova entrada como a última do bucket
    if (bucket_entry->after_idx == U32_MAX) {
        entry->before_idx = bucket_entry_idx;
        bucket_entry->after_idx = entry_idx;
        entry->after_idx = U32_MAX;
        return;
    }

    // se a entrada atual não for a última do bucket, insere a nova entrada depois dela e antes da próxima
    NameHashMapEntry* after_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, bucket_entry->after_idx);
    assert(after_entry != NULL, "after entry is null");

    entry->after_idx = bucket_entry->after_idx;
    entry->before_idx = bucket_entry_idx;
    bucket_entry->after_idx = entry_idx;
    after_entry->before_idx = entry_idx;
}

void name_hashmap_insert_entry_at_side_of_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_idx, usize bucket_entry_idx) {

    // se o index da entrada do bucket for U32_MAX, significa que o bucket está vazio, e a nova entrada deve ser a primeira
    if (bucket_entry_idx == U32_MAX) {
        *(usize*)vector_at(&map->buckets_vec, bucket_idx) = entry_idx;
        return;
    }
    
    // obtém a referência das entradas
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx);
    NameHashMapEntry* bucket_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, bucket_entry_idx);
    assert(entry != NULL && bucket_entry != NULL, "bucket entry is null");

    // se a nova entrada for menor que a atual, insere antes dela, se for maior, insere depois
    int strcmp_res = strcmp(entry->key, bucket_entry->key); // resultado da comparação
    assert(strcmp_res != 0, "entry equal to the one that would stay after or before it");
    if (strcmp_res < 0)
        name_hashmap_insert_entry_before_bucket_entry(map, entry_idx, bucket_entry_idx);
    else
        name_hashmap_insert_entry_after_bucket_entry(map, entry_idx, bucket_entry_idx);
}

void name_hashmap_update_indexes_for_entry(NameHashMap* map, NameHashMapEntry* entry, usize entry_new_idx) {

    // atualiza os índices da entrada seguinte
    if (entry->after_idx != U32_MAX) {
        NameHashMapEntry* after_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry->after_idx);
        assert(after_entry != NULL, "after entry is null");
        after_entry->before_idx = entry_new_idx;
    }

    // atualiza os índices da entrada anterior
    if (entry->before_idx != U32_MAX) {
        NameHashMapEntry* before_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry->before_idx);
        assert(before_entry != NULL, "before entry is null");
        before_entry->after_idx = entry_new_idx;
    } else {
        // se a entrada não tiver uma entrada anterior, significa que ela é a primeira do bucket
        // então, atualiza o índice do bucket para ela
        usize bucket_idx = name_hashmap_calc_bucket(map, entry->key);
        usize* bucket = (usize*)vector_at(&map->buckets_vec, bucket_idx);
        assert(bucket != NULL, "bucket is null");
        *bucket = entry_new_idx;
    }
}

bool name_hashmap_init_bucket_vec(NameHashMap* map, usize size, ErrorState* estate) {

    // inicializa o vetor de buckets
    vector_init(&map->buckets_vec, sizeof(usize), NULL);
    for (usize i = 0; i < size; i++) {
        
        usize empty_bucket_value = U32_MAX;
        if (!vector_insert(&map->buckets_vec, i, &empty_bucket_value, estate)) {
            vector_free(&map->buckets_vec);
            return false;
        }
    }

    return true;
}

void name_hashmap_rehash(NameHashMap* map) {
    
    // limpa as relações entre as entradas
    for (usize i = 0; i < vector_size(&map->entries_vec); i++) {

        // obtém a referência da entrada
        assert(i < vector_size(&map->entries_vec), "entry index out of bounds");
        NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, i);
        assert(entry != NULL, "entry is null");

        // atualiza os índices da entrada
        entry->before_idx = U32_MAX;
        entry->after_idx = U32_MAX;
    }

    // faz a re-inserção de todas as entradas
    for (usize i = 0; i < vector_size(&map->entries_vec); i++) {

        // obtém a referência da entrada
        NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, i);

        // insere a entrada no novo vetor de buckets
        usize bucket_idx = name_hashmap_calc_bucket(map, entry->key);
        
        
        // obtém o índice da entrada a qual essa deve ser vizinha
        usize entry_target_idx = U32_MAX;
        if (name_hashmap_find_key_entry_bucket_idx(map, entry->key, bucket_idx, &entry_target_idx))
            internal_error("key already exists");

        // insere a entrada no vetor de buckets
        name_hashmap_insert_entry_at_side_of_bucket_entry(map, i, bucket_idx, entry_target_idx);
    }
}

bool name_hashmap_grow(NameHashMap* map, ErrorState* estate) {

    // dobra o tamanho do vetor de buckets
    usize new_buckets_size = (usize)((f64)vector_size(&map->buckets_vec) * NAME_HASHMAP_GROW_FACTOR);
    vector_free(&map->buckets_vec);
    if (!name_hashmap_init_bucket_vec(map, new_buckets_size, estate))
        return false;

    // rehashing das entradas
    name_hashmap_rehash(map);
    return true;
}


/* ---------------------------- public functions ---------------------------- */

NameHashMapEntry* name_hashmap_entry_new(char const* key, void* value_ptr, bool copy_key, ErrorState* estate) {

    // alocação, criação, e inserção da nova entrada do map na última posição do vetor de entradas
    NameHashMapEntry* entry = malloc(sizeof(NameHashMapEntry)); // novo objeto de entrada
    if (entry == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    if (copy_key) {
        char* new_key = malloc(strlen(key) + 1); // buffer para a copia da chave
        if (new_key == NULL) {
            free(entry);
            set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
            return false;
        }
        strcpy(new_key, key);
        key = new_key;
    }

    entry->key = key;
    entry->value = value_ptr;
    entry->before_idx = U32_MAX;
    entry->after_idx = U32_MAX;
    return entry;
}

void name_hashmap_entry_free(NameHashMapEntry* entry, NameHashMap* map) {

    if (map->own_keys)
        free((void*)entry->key);

    if (entry->value != NULL && map->free_fn != NULL)
        map->free_fn(entry->value);
    
    free(entry);
}

bool name_hashmap_init(NameHashMap* map, void(*free_fn)(void*), bool own_keys, ErrorState* estate) {
    
    map->free_fn = free_fn;
    map->own_keys = own_keys;
    vector_init(&map->entries_vec, sizeof(NameHashMapEntry*), NULL);
    if (!name_hashmap_init_bucket_vec(map, NAME_HASHMAP_INIT_BUCKETS, estate)) {
        vector_free(&map->entries_vec);
        return false;
    }

    return true;
}

void name_hashmap_free(void* map) {

    NameHashMap* name_map = (NameHashMap*)map; // ponteiro convertido para o tipo correto
    vector_free(&name_map->buckets_vec);
    
    for (usize i = 0; i < vector_size(&name_map->entries_vec); i++) {
        NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&name_map->entries_vec, i); // endereço da entrada
        name_hashmap_entry_free(entry, map);
    }
    vector_free(&name_map->entries_vec);
}

bool name_hashmap_insert(NameHashMap* map, char const* key, void* value_ptr, ErrorState* estate) {

    // verifica se o hashmap precisa crescer
    if (vector_size(&map->entries_vec) >= (usize)((double)vector_size(&map->buckets_vec) * NAME_HASHMAP_MAX_LOAD_FACTOR))
        if (!name_hashmap_grow(map, estate))
            return false;

    // obtém o índice do bucket onde a nova entrada deve ser inserida
    usize bucket_idx = name_hashmap_calc_bucket(map, key);

    // verifica se a chave já existe, e se sim, retorna false
    // se não, obtém o índice da entrada da qual a nova deve ser vizinha
    usize entry_idx;
    if (name_hashmap_find_key_entry_bucket_idx(map, key, bucket_idx, &entry_idx)) {
        internal_error("key already exists");
        return false;
    }

    // cria a nova entrada
    NameHashMapEntry* entry = name_hashmap_entry_new(key, value_ptr, map->own_keys, estate);
    assert(entry != NULL, "failed to create new entry");

    // inserindo a nova entrada na última posição do vetor de entradas
    usize new_entry_idx = vector_size(&map->entries_vec);
    if (!vector_insert(&map->entries_vec, new_entry_idx, &entry, estate))
        return false;

    name_hashmap_insert_entry_at_side_of_bucket_entry(map, new_entry_idx, bucket_idx, entry_idx);
    return true;
}

void handle_removed_entry(NameHashMap* map, NameHashMapEntry* entry) {

    // se a entrada não for a última do bucket, atualiza o índice da entrada seguinte
    if (entry->after_idx != U32_MAX) {
        NameHashMapEntry* after_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry->after_idx);
        assert(after_entry != NULL, "after entry is null");
        after_entry->before_idx = entry->before_idx;
    }

    // se a entrada for a primeira do bucket, atualiza o índice do bucket
    if (entry->before_idx == U32_MAX) {
        usize bucket_idx = name_hashmap_calc_bucket(map, entry->key);
        usize* bucket = (usize*)vector_at(&map->buckets_vec, bucket_idx);
        assert(bucket != NULL, "bucket is null");
        *bucket = entry->after_idx;
    } else {
        // se a entrada não for a primeira do bucket, atualiza o índice da entrada anterior
        NameHashMapEntry* before_entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry->before_idx);
        assert(before_entry != NULL, "before entry is null");
        before_entry->after_idx = entry->after_idx;
    }
}

void name_hashmap_shrink_if_possible(NameHashMap* map) {

    if (vector_size(&map->buckets_vec) > NAME_HASHMAP_INIT_BUCKETS * NAME_HASHMAP_GROW_FACTOR)
        return;

    // verifica se o hashmap pode encolher
    if (vector_size(&map->entries_vec) <= (usize)(((double)vector_size(&map->buckets_vec) * NAME_HASHMAP_MAX_LOAD_FACTOR) / NAME_HASHMAP_GROW_FACTOR / 2)) {
        vector_free(&map->buckets_vec);
        name_hashmap_init_bucket_vec(map, vector_size(&map->buckets_vec) / NAME_HASHMAP_GROW_FACTOR, NULL);
        name_hashmap_rehash(map);
    }
}

void* name_hashmap_remove(NameHashMap* map, char const* key, ErrorState* estate) {

    // encontra o bucket onde a chave pode estar
    usize bucket_idx = name_hashmap_calc_bucket(map, key);

    // encontra a posição do elemento a ser removido
    // e verifica se a chave existe
    usize idx;
    if (!name_hashmap_find_key_entry_bucket_idx(map, key, bucket_idx, &idx))
        return NULL;

    // verifica se a entrada existe
    if (idx == U32_MAX)
        return NULL;

    // remove a entrada do vetor de entradas
    // move a última entrada do vetor para o lugar da entrada removida
    // e atualiza os índices dela
    NameHashMapEntry* entry;
    usize last_entries_vec_idx = vector_size(&map->entries_vec) - 1;
    if (idx != last_entries_vec_idx) {
        
        name_hashmap_update_indexes_for_entry(map, *(NameHashMapEntry**)vector_at(&map->entries_vec, last_entries_vec_idx), idx);
        
        NameHashMapEntry* last_entry;
        if (!vector_remove(&map->entries_vec, last_entries_vec_idx, &last_entry, estate))
            return NULL;
        
        NameHashMapEntry** entry_ptr = (NameHashMapEntry**)vector_at(&map->entries_vec, idx); // endereço da entrada a ser removida
        entry = *entry_ptr;
        *entry_ptr = last_entry;
        
    } else {
        if (!vector_remove(&map->entries_vec, last_entries_vec_idx, &entry, estate))
            return NULL;
    }

    // atualiza os índices da entrada removida
    handle_removed_entry(map, entry);

    // libera a entrada e retorna o valor dela
    void* value = entry->value; // valor a ser retornado
    entry->value = NULL;
    name_hashmap_entry_free(entry, map);
    name_hashmap_shrink_if_possible(map);
    return value;
}

void* name_hashmap_find(NameHashMap* map, char const* key) {

    // encontra o bucket onde a chave pode estar
    usize bucket_idx = name_hashmap_calc_bucket(map, key);

    // encontra a posição do elemento a ser removido
    // e verifica se a chave existe
    usize idx;
    if (!name_hashmap_find_key_entry_bucket_idx(map, key, bucket_idx, &idx))
        return NULL;

    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, idx); // endereço da entrada
    assert(entry != NULL, "entry is null");
    return entry->value;
}

bool name_hashmap_contains(NameHashMap* map, char const* key) {
    return name_hashmap_find(map, key) != NULL;
}

usize name_hashmap_size(NameHashMap const* map) {
    return vector_size(&map->entries_vec);
}

void* name_hashmap_get_by_entry_idx(NameHashMap* map, usize entry_idx) {
    
    assert(entry_idx < vector_size(&map->entries_vec), "entry index out of bounds");
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx); // endereço da entrada
    debug_assert(entry != NULL, "entry is null");
    
    return entry->value;
}

char* name_hashmap_get_key_by_entry_idx(NameHashMap* map, usize entry_idx) {

    assert(entry_idx < vector_size(&map->entries_vec), "entry index out of bounds");
    NameHashMapEntry* entry = *(NameHashMapEntry**)vector_at(&map->entries_vec, entry_idx); // endereço da entrada
    debug_assert(entry != NULL, "entry is null");
    
    return (char*)entry->key;
}
