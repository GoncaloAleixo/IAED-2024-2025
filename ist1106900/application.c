/**
 * @file application.c
 * @author ist1106900 (Goncalo Aleixo)
*/

// header
#include "application.h"

// local
#include "core.h"
#include "data_types.h"

// c stdlib
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/* -------------------------------------------------------------------------- */
/*                                    batch                                   */
/* -------------------------------------------------------------------------- */

Batch* batch_create(char* name_ref, char* vaccine_name_ref, Date expiration_date, u32 quantity, ErrorState* estate) {

    // alocação da memória onde o batch será armazenado
    Batch* batch = malloc(sizeof(Batch)); // endereço onde o batch será armazenado
    if (batch == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }

    // cópia do nome do batch, para garantir ownership da memória
    batch->name = malloc(strlen(name_ref) + 1);
    if (batch->name == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }
    strcpy(batch->name, name_ref);

    // inicialização dos campos do batch
    batch->vaccine_name_ref = vaccine_name_ref;
    batch->expiration_date = expiration_date;
    batch->quantity = quantity;
    batch->used = 0;
    batch->retired = false;
    return batch;
}

void batch_free(void* _batch) {

    Batch* batch = (Batch*)_batch; // ponteiro convertido para o tipo Batch
    free(batch->name);
    free(batch);
}

i8 batch_cmp_ptr(void* _a, void* _b, void* ctx) {

    (void)ctx;
    Batch* a = *(Batch**)_a; // ponteiro convertido para o tipo Batch
    Batch* b = *(Batch**)_b; // ponteiro convertido para o tipo Batch

    // implementa a ordenação dos batches por data de expiração, e em caso de empate, por nome

    i8 date_cmp_res = date_cmp(&a->expiration_date, &b->expiration_date); // resultado da comparação
    if (date_cmp_res != 0)
        return date_cmp_res;

    return (i8)strcmp(a->name, b->name);
}

usize find_new_batch_list_idx(Vector* batch_list, Batch* batch) {

    bool found; // resultado da busca, flag que indica se o batch foi encontrado
    usize idx = vector_binary_search(batch_list, &batch, &found, batch_cmp_ptr, NULL); // resultado da busca, index do batch
    assert(found == false, "batch already exists in batch list");

    return idx;
}

usize find_batch_list_idx(Vector* batch_list, Batch* batch) {

    bool found; // resultado da busca, flag que indica se o batch foi encontrado
    usize idx = vector_binary_search(batch_list, &batch, &found, batch_cmp_ptr, NULL); // resultado da busca, index do batch
    assert(found == true, "batch does not exist in batch list");

    return idx;
}



/* -------------------------------------------------------------------------- */
/*                                   vaccine                                  */
/* -------------------------------------------------------------------------- */

Vaccine* vaccine_create(char* name, ErrorState* estate) {

    // alocação da memória onde a vacina será armazenada
    Vaccine* vaccine = malloc(sizeof(Vaccine)); // endereço onde a vacina será armazenada
    if (vaccine == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }

    // cópia do nome da vacina, para garantir ownership da memória
    vaccine->name = malloc(strlen(name) + 1);
    if (vaccine->name == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }
    strcpy(vaccine->name, name);

    // inicialização dos membros da vacina
    if (!name_hashmap_init(&vaccine->batches, batch_free, true, estate)) {
        free(vaccine->name);
        free(vaccine);
        return NULL;
    }
    vector_init(&vaccine->batch_list, sizeof(Batch*), NULL);
    vaccine->oldest_valid_batch_idx = USIZE_MAX;
    return vaccine;
}

void vaccine_free(void* _vaccine) {

    Vaccine* vaccine = (Vaccine*)_vaccine; // ponteiro convertido para o tipo Vaccine

    // libera a memória do nome, lista ordenada de batches, e map de batches, antes de liberar a memória da vacina
    free(vaccine->name);
    name_hashmap_free(&vaccine->batches);
    vector_free(&vaccine->batch_list);
    free(vaccine);
}

bool vaccine_add_batch(Vaccine* vaccine, Batch* batch, Date* current_date, ErrorState* estate) {

    // verifica se o batch já existe na vacina
    if (name_hashmap_contains(&vaccine->batches, batch->name)) {
        batch_free(batch);
        set_error(ERROR_DUPLICATE_BATCH_NUMBER, estate, NULL, NULL, NULL);
        return false;
    }

    // insere o batch no map de batches
    if (!name_hashmap_insert(&vaccine->batches, batch->name, batch, estate))
        return false;

    // cria uma referência para o batch, que será mantida numa lista ordenada, para facilitar a busca
    if (!vector_insert(&vaccine->batch_list, find_new_batch_list_idx(&vaccine->batch_list, batch), &batch, estate))
        return false;

    vaccine_update_oldest_valid_batch_idx(vaccine, current_date);
    return true;
}

bool vaccine_remove_batch(Vaccine* vaccine, Batch* batch, ErrorState* estate) {

    // verifica se o batch existe na vacina
    assert(name_hashmap_contains(&vaccine->batches, batch->name), "batch does not exist in vaccine");

    // remove o batch do map de batches
    if (!name_hashmap_remove(&vaccine->batches, batch->name, estate)) {
        set_error(ERROR_NO_SUCH_BATCH, estate, NULL, NULL, NULL);
        return false;
    }

    // remove o batch da lista ordenada de batches
    if (!vector_remove(&vaccine->batch_list, find_batch_list_idx(&vaccine->batch_list, batch), NULL, estate))
        return false;

    batch_free(batch);
    return true;
}

void update_oldest_valid_batch_idx(Vaccine* vaccine, Date* current_date) {
    
    // primeiro, procura o lote com a mesma data com o menor índice
    for (usize j = vaccine->oldest_valid_batch_idx; j < vector_size(&vaccine->batch_list); --j) {
        Batch* batch = *(Batch**)vector_at(&vaccine->batch_list, j); // endereço do lote
        if (date_cmp(current_date, &batch->expiration_date) > 1) {
            vaccine->oldest_valid_batch_idx = j;
            break;
        }
    }

    // partindo do primeiro lote com a mesma data, verifica se existe um lote válido, com data igual ou superior à data atual
    bool found_new_valid_batch = false; // flag que indica se um lote válido foi encontrado
    for (usize j = vaccine->oldest_valid_batch_idx; j < vector_size(&vaccine->batch_list); ++j) {
        
        Batch* batch = *(Batch**)vector_at(&vaccine->batch_list, j); // endereço do lote
        if ((date_cmp(current_date, &batch->expiration_date) <= 0) && batch->used < batch->quantity && !batch->retired) {
            found_new_valid_batch = true;
            vaccine->oldest_valid_batch_idx = j;
            break;
        }
    }

    // se não foi encontrado um novo lote válido, atualiza o index de último lote válido para USIZE_MAX
    if (!found_new_valid_batch) vaccine->oldest_valid_batch_idx = USIZE_MAX;
}

void find_oldest_valid_batch(Vaccine* vaccine, Date* current_date) {
    
    // se não existia um lote válido, procura o primeiro válido lote com a mesma data
    for (usize j = 0; j < vector_size(&vaccine->batch_list); ++j) {
        Batch* batch = *(Batch**)vector_at(&vaccine->batch_list, j); // endereço do lote
        if ((date_cmp(current_date, &batch->expiration_date) <= 0) && batch->used < batch->quantity && !batch->retired) {
            vaccine->oldest_valid_batch_idx = j;
            break;
        }
    }
}

void vaccine_update_oldest_valid_batch_idx(Vaccine* vaccine, Date* current_date) {

    if (vaccine->oldest_valid_batch_idx != USIZE_MAX)
        update_oldest_valid_batch_idx(vaccine, current_date);
    else
        find_oldest_valid_batch(vaccine, current_date);
}



/* -------------------------------------------------------------------------- */
/*                                  injection                                 */
/* -------------------------------------------------------------------------- */

Injection* injection_create(char* utente_name, char* batch_name, Date date, u64 utente_id, ErrorState* estate) {

    Injection* new_injection = (Injection*)malloc(sizeof(Injection)); // endereço onde a injeção será armazenada
    if (new_injection == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }

    new_injection->utente_name = utente_name;
    new_injection->batch_name = batch_name;
    new_injection->date = date;
    new_injection->removed = false;
    new_injection->info_string = NULL;
    injection_get_info_string(new_injection, utente_id, estate);
    return new_injection;
}


void injection_free(void* _injection) {

    Injection* injection = (Injection*)_injection; // ponteiro convertido para o tipo Injection
    free(injection->info_string);
    free(injection);
}

usize usize_decimal_length(usize num) {
    if (num == 0)
        return 1;

    usize length = 0; // tamanho do número
    while (num > 0) {
        num /= 10;
        length++;
    }
    return length;
}

char* injection_get_info_string(Injection* injection, u64 utente_id, ErrorState* estate) {

    
    if (injection->info_string != NULL)
        return injection->info_string;
    
    usize injection_key_size = 0; // tamanho da string de informação
    injection_key_size += usize_decimal_length(utente_id);
    injection_key_size += 1; // separador
    injection_key_size += strlen(injection->batch_name);
    injection_key_size += 1; // separador
    injection_key_size += 2; // two digits for day
    injection_key_size += 2; // two digits for month
    injection_key_size += usize_decimal_length(injection->date.year);

    char* injection_info_string = (char*)malloc(injection_key_size + 1); // endereço onde a string de informação será armazenada
    if (injection_info_string == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    sprintf(injection_info_string, "%llu-%s-%02u%02u%u", utente_id, injection->batch_name, injection->date.day, injection->date.month, injection->date.year);
    injection->info_string = injection_info_string;
    return injection_info_string;
}



/* -------------------------------------------------------------------------- */
/*                                 application                                */
/* -------------------------------------------------------------------------- */

bool application_init(Application* app, ErrorState* estate) {
    
    if (!name_hashmap_init(&app->injection_map, injection_free, false, estate))
        return false;
    
    
    if (!name_hashmap_init(&app->vaccine_map, vaccine_free, true, estate)) {
        return false;
    }

    if (!name_hashmap_init(&app->batch_name_to_vaccine_name_map, NULL, false, estate)) {
        name_hashmap_free(&app->vaccine_map);
        return false;
    }

    if (!name_hashmap_init(&app->utente_map, utente_free, true, estate)) {
        name_hashmap_free(&app->batch_name_to_vaccine_name_map);
        name_hashmap_free(&app->vaccine_map);
        return false;
    }
    
    // inicializa a data atual
    app->current_date.day = START_DATE_DAY;
    app->current_date.month = START_DATE_MONTH;
    app->current_date.year = START_DATE_YEAR;

    // inicializa o contador de ids de utentes
    app->utente_id_counter = 0;
    
    return true;
}

void application_free(Application* app) {

    // libera todos os dados alocados dinamicamente
    // os vetores e mapas são liberados em ordem inversa de alocação
    // não é necessário liberar o próprio objeto já que ele é alocado na 'stack'
    name_hashmap_free(&app->utente_map);
    name_hashmap_free(&app->batch_name_to_vaccine_name_map);
    name_hashmap_free(&app->vaccine_map);
    name_hashmap_free(&app->injection_map);
}

bool new_vaccine(Application* app, char* vaccine_name, ErrorState* estate) {

    // verifica se o número máximo de vacinas foi atingido
    if (name_hashmap_size(&app->vaccine_map) >= MAX_VACCINE_NUMBER) {
        set_error(ERROR_TOO_MANY_VACCINES, estate, NULL, NULL, NULL);
        return false;
    }

    Vaccine* new_vaccine = vaccine_create(vaccine_name, estate); // nova vacina a ser inserida
    if (new_vaccine == NULL)
        return false;

    // verifica se a vacina já existe
    // caso contrário, insere a nova vacina no mapa de vacinas
    if (name_hashmap_find(&app->vaccine_map, vaccine_name) != NULL) {
        vaccine_free(new_vaccine);
        internal_error("INTERNAL ERROR: trying to create a vaccine that already exists");
        return false;
    } else {
        
        if (!name_hashmap_insert(&app->vaccine_map, vaccine_name, new_vaccine, estate))
            return false;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/*                                   utente                                   */
/* -------------------------------------------------------------------------- */


void utente_date_injection_key_list_free(void* _key_list) {
    
    UtenteDateInjectionList* key_list = (UtenteDateInjectionList*)_key_list; // ponteiro convertido para o tipo UtenteDateInjectionList
    vector_free(&key_list->injection_key_list);
}

Utente* utente_create(char* name, u64 id, ErrorState* estate) {

    Utente* new_utente = (Utente*)malloc(sizeof(Utente)); // endereço onde o utente será armazenado
    if (new_utente == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return NULL;
    }

    // inicializa o id do utente
    new_utente->id = id;

    // inicializa o número de injeções aplicadas
    new_utente->injection_count = 0;

    // faz a cópia do nome do utente, garantindo o ownership da memória
    new_utente->name = name;

    // inicializa o vetor de datas de aplicações
    vector_init(&new_utente->vaccine_last_injection_key_list, sizeof(VaccineNameLastInjectionKey), NULL);
    
    // inicializa o vetor de vacinas aplicadas
    vector_init(&new_utente->date_injection_key_list, sizeof(UtenteDateInjectionList), utente_date_injection_key_list_free);

    return new_utente;
}

void utente_free(void* _utente) {

    Utente* utente = (Utente*)_utente; // ponteiro convertido para o tipo Utente

    vector_free(&utente->vaccine_last_injection_key_list);
    vector_free(&utente->date_injection_key_list);
    free(utente);
}

i8 utente_date_injection_key_list_cmp(void* _a, void* _b, void* ctx) {
    
    (void)ctx; // unused parameter
    UtenteDateInjectionList* a = (UtenteDateInjectionList*)_a; // ponteiro convertido para o tipo UtenteDateInjectionList
    Date* b = (Date*)_b;  // ponteiro convertido para o tipo Date

    return date_cmp(&a->date, b);
}

i8 utente_vaccine_last_injection_key_list_cmp(void* _a, void* _b, void* ctx) {
    
    (void)ctx; // unused parameter
    VaccineNameLastInjectionKey* a = (VaccineNameLastInjectionKey*)_a; // ponteiro convertido para o tipo VaccineNameLastInjectionKey
    char* b = (char*)_b; // ponteiro convertido para o tipo char*

    return (i8)strcmp(a->vaccine_name, b);
}


bool utente_add_injection_at_vaccine_name_last_injection_key_list(Utente* utente, Application* app, Injection* injection, char* injection_key, ErrorState* estate) {

    char* vaccine_name = (char*)name_hashmap_find(&app->batch_name_to_vaccine_name_map, injection->batch_name); // nome da vacina

    // verifica se a vacina já existe
    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->vaccine_last_injection_key_list, vaccine_name, &found, utente_vaccine_last_injection_key_list_cmp, NULL); // index resultante
    if (!found) {

        // cria uma nova vacina
        VaccineNameLastInjectionKey new_vaccine_last_injection_key; // nova entrada a ser inserida
        new_vaccine_last_injection_key.vaccine_name = vaccine_name;
        new_vaccine_last_injection_key.injection_key = NULL;

        // insere a nova vacina no vetor de vacinas aplicadas
        if (!vector_insert(&utente->vaccine_last_injection_key_list, idx, &new_vaccine_last_injection_key, estate))
            return false;
    }

    // atualiza a chave da última aplicação da vacina
    VaccineNameLastInjectionKey* vaccine_last_injection = (VaccineNameLastInjectionKey*)vector_at(&utente->vaccine_last_injection_key_list, idx); // entrada a ser atualizada
    assert(vaccine_last_injection != NULL, "vaccine injections list not found");
    vaccine_last_injection->injection_key = injection_key;

    return true;
}

bool utente_add_date_injection_key_list_vector(Utente* utente, Date* date, Injection* injection, ErrorState* estate) {
    
    // cria uma nova data de aplicação
    UtenteDateInjectionList new_date_injection_key_list; // nova entrada na lista da datas
    new_date_injection_key_list.date = injection->date;
    vector_init(&new_date_injection_key_list.injection_key_list, sizeof(char*), NULL);

    // insere a nova data de aplicação no vetor de datas de aplicações
    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    assert(!found, "date injection list already exists");

    if (!vector_insert(&utente->date_injection_key_list, idx, &new_date_injection_key_list, estate))
        return false;

    return true;
}

bool utente_add_injection_at_injection_date_injection_list_map(Utente* utente, Injection* injection, char* injection_key, ErrorState* estate) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, &injection->date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    if (!found) {
        if (!utente_add_date_injection_key_list_vector(utente, &injection->date, injection, estate))
            return false;
    }
    
    UtenteDateInjectionList* utente_date_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // entrada a ser atualizada
    debug_assert(utente_date_key_list != NULL, "injection date list not found");

    // atualiza a lista de aplicações do dia
    if (!vector_insert(&utente_date_key_list->injection_key_list, vector_size(&utente_date_key_list->injection_key_list), &injection_key, estate))
        return false;

    return true;
}

bool utente_add_injection(Utente* utente, Application* app, char* injection_key, ErrorState* estate) {

    // insere a aplicação no mapa de vacinas aplicadas
    Injection* injection = name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser inserida
    assert(injection != NULL, "injection is null");
    
    // vaccine_name_last_injection_key_map
    if (!utente_add_injection_at_vaccine_name_last_injection_key_list(utente, app, injection, injection_key, estate))
        return false;

    // injection_date_day_data_map
    if (!utente_add_injection_at_injection_date_injection_list_map(utente, injection, injection_key, estate))
        return false;

    utente->injection_count++;

    return true;
}

void utente_remove_injection_at_vaccine_name_last_injection_key(Utente* utente, char* vaccine_name, char* injection_key) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->vaccine_last_injection_key_list, vaccine_name, &found, utente_vaccine_last_injection_key_list_cmp, NULL); // index resultante
    if (found) {
        VaccineNameLastInjectionKey* vaccine_last_injection = (VaccineNameLastInjectionKey*)vector_at(&utente->vaccine_last_injection_key_list, idx); // entrada a ser atualizada
        assert(vaccine_last_injection != NULL, "vaccine injections list not found");
        if (strcmp(vaccine_last_injection->injection_key, injection_key) == 0)
            vaccine_last_injection->injection_key = NULL;
    }
}

void utente_remove_injection_at_date_injection_key_list(Utente* utente, char* injection_key, Injection* injection, ErrorState* estate) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, &injection->date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    assert(found, "injection date list not found");

    UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // entrada a ser atualizada

    bool removed = false; // flag que indica se a aplicação foi removida
    for (usize i = 0; i < vector_size(&utente_injection_key_list->injection_key_list); ++i) {
        
        char* injection_key_ptr = *(char**)vector_at(&utente_injection_key_list->injection_key_list, i); // key da aplicação
        if (strcmp(injection_key_ptr, injection_key) == 0) {
            vector_remove(&utente_injection_key_list->injection_key_list, i, NULL, estate);
            removed = true;
            break;
        }
    }
    assert(removed, "injection key not found in injection date list");
}

void utente_remove_injection(Utente* utente, Application* app, char* injection_key, ErrorState* estate) {

    Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser removida
    assert(injection != NULL, "injection is null");
    char* vaccine_name = (char*)name_hashmap_find(&app->batch_name_to_vaccine_name_map, injection->batch_name); // nome da vacina
    
    /* --------------------- vaccine_last_injection_key_list -------------------- */
    utente_remove_injection_at_vaccine_name_last_injection_key(utente, vaccine_name, injection_key);

    /* ----------------------- injection_date_day_data_map ---------------------- */
    utente_remove_injection_at_date_injection_key_list(utente, injection_key, injection, estate);
    
    /* --------------- decrementa o número de aplicações do utente -------------- */
    utente->injection_count--;

    // se não houver mais aplicações no dia, remove o dia do mapa de dias
    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, &injection->date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    assert(found, "injection date list not found");
    UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // entrada a ser atualizada
    if (utente_injection_key_list->injection_key_list.size == 0) {
        vector_remove(&utente->date_injection_key_list, idx, NULL, estate);
        return;
    }
}

bool utente_remove_entire_day_data(Utente* utente, Application* app, Date* date, ErrorState* estate) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    assert(found, "injection date list not found");

    UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // entrada a ser atualizada
    usize injection_count = vector_size(&utente_injection_key_list->injection_key_list); // número de aplicações do dia

    // itera por todas as aplicações do dia e remove elas do mapa de aplicações
    for (usize i = 0; i < vector_size(&utente_injection_key_list->injection_key_list); ++i) {
        
        char* injection_key = *(char**)vector_at(&utente_injection_key_list->injection_key_list, i); // key da aplicação
        Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser removida
        assert(injection != NULL, "injection is null");

        char* vaccine_name = (char*)name_hashmap_find(&app->batch_name_to_vaccine_name_map, injection->batch_name); // nome da vacina
        
        bool found; // indica se a busca foi bem sucedida
        usize idx = vector_binary_search(&utente->vaccine_last_injection_key_list, vaccine_name, &found, utente_vaccine_last_injection_key_list_cmp, NULL); // index resultante
        if (found) {

            VaccineNameLastInjectionKey* vaccine_last_injection = (VaccineNameLastInjectionKey*)vector_at(&utente->vaccine_last_injection_key_list, idx); // entrada a ser atualizada
            assert(vaccine_last_injection != NULL, "vaccine injections list not found");

            if (strcmp(vaccine_last_injection->injection_key, injection_key) == 0)
                vaccine_last_injection->injection_key = NULL;
        }
    }

    // remove o dia do utente
    vector_remove(&utente->date_injection_key_list, idx, NULL, estate);

    utente->injection_count -= injection_count;
    return true;
}


void application_show_batch(Batch* batch) {

    // imprime os dados de um lote
    // compartilhada entre as instruções de listar vacinas e listar aplicações

    printf("%s ", batch->vaccine_name_ref);
    printf("%s ", batch->name);
    date_put(&batch->expiration_date);
    printf(" %u ", batch->quantity - batch->used);
    printf("%u\n", batch->used);
}

/* -------------------------------------------------------------------------- */
/*                            instruction handling                            */
/* -------------------------------------------------------------------------- */

bool handle_instruction_criar_lote(Application* app, InstructionBody_CriarLote* body, ErrorState* estate) {

    // checa se a data de validade é superior à data atual
    if (date_cmp(&body->expiration_date, &app->current_date) == -1) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        return false;
    }

    // verifica se já existe um lote com o mesmo nome
    if (name_hashmap_find(&app->batch_name_to_vaccine_name_map, body->batch_name) != NULL) {
        set_error(ERROR_DUPLICATE_BATCH_NUMBER, estate, NULL, NULL, NULL);
        return false;
    }

    // verifica se a vacina já existe
    // caso contrário, cria uma nova vacina
    if (name_hashmap_find(&app->vaccine_map, body->vaccine_name) == NULL)
        if (!new_vaccine(app, body->vaccine_name, estate))
            return false;

    Vaccine* vaccine = (Vaccine*)name_hashmap_find(&app->vaccine_map, body->vaccine_name); // vacina a receber o lote
    assert(vaccine != NULL, "could not find a vaccine after creating it");


    // criação do novo lote
    Batch* new_batch = batch_create(body->batch_name, vaccine->name, body->expiration_date, body->dose_count, estate); // novo lote
    if (new_batch == NULL)
        return false;

    // insere o novo lote na vacina
    if (!vaccine_add_batch(vaccine, new_batch, &app->current_date, estate))
        return false;

    // insere referências para o novo lote nas estruturas da aplicação para facilitar buscas
    if (!name_hashmap_insert(&app->batch_name_to_vaccine_name_map, new_batch->name, vaccine->name, estate))
        return false;

    // imprime o nome do lote
    printf("%s\n", new_batch->name);
    return true;
}

bool get_vaccine_by_name(Application* app, char* vaccine_name, Vaccine** dst_vaccine, ErrorState* estate) {

    Vaccine* vaccine = (Vaccine*)name_hashmap_find(&app->vaccine_map, vaccine_name); // vacina sendo buscada
    if (vaccine == NULL) {
        set_error(ERROR_NO_SUCH_VACCINE, estate, NULL, vaccine_name, NULL);
        return false;
    }

    *dst_vaccine = vaccine;
    return true;
}

bool handle_instruction_listar_vacinas_utente(Application* app, InstructionBody_ListarVacinas* body, ErrorState* estate) {

    for (usize i = 0; i < body->vaccine_names.size; ++i) {
         
        // obtém a vacina pelo nome, verificando se ela existe
        char* vaccine_name = *(char**)vector_at(&body->vaccine_names, i); // nome da vacina
        Vaccine* vaccine; // ponteiro onde será colocado o endereço da vacina
        if (!get_vaccine_by_name(app, vaccine_name, &vaccine, estate)) {
            if (estate->return_code == ERROR_NO_SUCH_VACCINE)
                continue;
            else
                return false;
        }

        // itera por todos os lotes da vacina e os imprime
        for (usize j = 0; j < vector_size(&vaccine->batch_list); ++j) {
            Batch* batch = *(Batch**)vector_at(&vaccine->batch_list, j); // endereço do lote
            application_show_batch(batch);
        }
    }

    return true;
}

bool handle_instruction_listar_vacinas(Application* app, InstructionBody_ListarVacinas* body, ErrorState* estate) {

    // caso o usuário tenha passado o nome de vacinas específicas, apenas essas vacinas serão listadas
    if (body->vaccine_names.size > 0) {
        if (!handle_instruction_listar_vacinas_utente(app, body, estate))
            return false;
    } else {

        Vector batches; // lista de lotes a serem removidos
        vector_init(&batches, sizeof(Batch*), NULL);

        // caso contrário, lotes de todas as vacinas serão listados
        for (usize i = 0; i < name_hashmap_size(&app->batch_name_to_vaccine_name_map); ++i) {
            
            char* vaccine_name = (char*)name_hashmap_get_by_entry_idx(&app->batch_name_to_vaccine_name_map, i); // nome da vacina
            char* batch_name = (char*)name_hashmap_get_key_by_entry_idx(&app->batch_name_to_vaccine_name_map, i); // nome do lote
            Vaccine* vaccine = (Vaccine*)name_hashmap_find(&app->vaccine_map, vaccine_name); // endereço da vacina
            Batch* batch = (Batch*)name_hashmap_find(&vaccine->batches, batch_name); // endereço do lote

            // lotes manualmente retirados não são listados, exceto se já possuírem doses aplicadas
            if (batch->retired && (batch->used == 0))
                continue;
            
            if (!vector_insert(&batches, find_new_batch_list_idx(&batches, batch), &batch, estate))
                return false;
        }

        for (usize i = 0; i < vector_size(&batches); ++i) {
            Batch* batch = *(Batch**)vector_at(&batches, i); // endereço do lote
            application_show_batch(batch);
        }

        vector_free(&batches);
    }


    return true;
}

bool handle_instruction_aplicar_dose_preparer(Application* app, InstructionBody_AplicarDose* body, Vaccine** out_vaccine, Batch** out_batch, ErrorState* estate) {

    // verifica se a vacina existe
    // caso especial, onde ao invés de retornar um erro de vacina inexiste, retornamos um erro de indisponibilidade de estoque
    Vaccine* vaccine = (Vaccine*)name_hashmap_find(&app->vaccine_map, body->vaccine_name); // vacina a ser aplicada
    if (vaccine == NULL) {
        set_error(ERROR_NO_STOCK, estate, NULL, NULL, NULL);
        return false;
    }

    if (vaccine->oldest_valid_batch_idx == USIZE_MAX) {
        // caso não tenha sido possível encontrar um lote disponível, retorna um erro
        set_error(ERROR_NO_STOCK, estate, NULL, NULL, NULL);
        return false;
    }
        
    Batch* batch = *(Batch**)vector_at(&vaccine->batch_list, vaccine->oldest_valid_batch_idx); // endereço do lote
    assert(batch != NULL, "batch is null");

    // verifica se o utente já existe, caso contrário, cria um novo utente
    if (!name_hashmap_contains(&app->utente_map, body->utente_name)) {

        Utente* utente = utente_create(body->utente_name, app->utente_id_counter++, estate); // novo utente
        if (utente == NULL)
            return false;
        
        if (!name_hashmap_insert(&app->utente_map, body->utente_name, utente, estate)) {
            utente_free(utente);
            return false;
        }

        utente->name = name_hashmap_get_key_by_entry_idx(&app->utente_map, name_hashmap_size(&app->utente_map) - 1);
    }

    *out_vaccine = vaccine;
    *out_batch = batch;
    return true;
}

bool check_utente_injection_compatibility(Application* app, Utente* utente, char* vaccine_name, ErrorState* estate) {

    // verifica se o utente já foi vacinado com a vacina nesse mesmo dia, retornando um erro caso positivo
    // primeiro verifica se o utente já possui aplicações da vacina
    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->vaccine_last_injection_key_list, vaccine_name, &found, utente_vaccine_last_injection_key_list_cmp, NULL); // index resultante
    if (!found)
        return true;

    VaccineNameLastInjectionKey* vaccine_last_injection = (VaccineNameLastInjectionKey*)vector_at(&utente->vaccine_last_injection_key_list, idx); // entrada a ser atualizada
    assert(vaccine_last_injection != NULL, "vaccine injections list not found");
    char* last_injection_key = vaccine_last_injection->injection_key; // key da última aplicação

    if (last_injection_key == NULL)
        return true;

    Injection* last_injection = (Injection*)name_hashmap_find(&app->injection_map, last_injection_key); // aplicação a ser verificada
    assert(last_injection != NULL, "last injection is null");

    // verifica se a data da última aplicação é igual à data atual
    if (date_cmp(&last_injection->date, &app->current_date) == 0) {
        set_error(ERROR_ALREADY_VACCINATED, estate, NULL, NULL, NULL);
        return false;
    }

    return true;
}

bool handle_instruction_aplicar_dose(Application* app, InstructionBody_AplicarDose* body, ErrorState* estate) {

    Vaccine* vaccine; Batch* batch; // ponteiros para a vacina e o lote a serem aplicados
    if (!handle_instruction_aplicar_dose_preparer(app, body, &vaccine, &batch, estate)) return false;

    // obtém o utente a partir do nome
    Utente* utente = (Utente*)name_hashmap_find(&app->utente_map, body->utente_name); // utente ao qual a vacina será aplicada
    if (utente == NULL) return false;

    // verifica se o utente já foi vacinado com a vacina nesse mesmo dia
    if (!check_utente_injection_compatibility(app, utente, body->vaccine_name, estate)) return false;
    
    // cria uma nova aplicação e a insere no mapa de aplicações
    Injection* injection = injection_create(utente->name, batch->name, app->current_date, utente->id, estate); // nova aplicação
    if (injection == NULL) return false;

    char* injection_info_string = injection->info_string; // string de informação da aplicação
    if (injection_info_string == NULL) {
        injection_free(injection);
        return false;
    }

    if (!name_hashmap_insert(&app->injection_map, injection_info_string, injection, estate)) {
        free(injection_info_string);
        injection_free(injection);
        return false;
    }

    // atualiza o utente com a nova aplicação
    if (!utente_add_injection(utente, app, injection_info_string, estate)) return false;

    // imprime o nome do lote e atualiza a quantidade de doses utilizadas
    printf("%s\n", batch->name);
    
    batch->used += 1;
    if (batch->used == batch->quantity)
        vaccine_update_oldest_valid_batch_idx(vaccine, &app->current_date);
    
    return true;
    
}

bool handle_instruction_retirar_lote(Application* app, InstructionBody_RetirarLote* body, ErrorState* estate) {

    // obtém o nome da vacina a partir do nome do lote, retornando erro caso não exista lote com esse nome
    char* vaccine_name = (char*)name_hashmap_find(&app->batch_name_to_vaccine_name_map, body->batch_name); // nome da vacina
    if (vaccine_name == NULL) {
        set_error(ERROR_NO_SUCH_BATCH, estate, body->batch_name, NULL, NULL);
        return false;
    }

    // obtém a vacina a partir do nome
    Vaccine* vaccine = (Vaccine*)name_hashmap_find(&app->vaccine_map, vaccine_name); // vacina da qual o lote será retirado
    assert(vaccine != NULL, "vaccine is null");

    // obtém o lote a partir do nome
    Batch* batch = (Batch*)name_hashmap_find(&vaccine->batches, body->batch_name); // lote a ser retirado
    assert(batch != NULL, "batch is null");

    // atualiza o status do lote e imprime a quantidade de doses retiradas
    batch->quantity = batch->used;
    batch->retired = true;
    vaccine_update_oldest_valid_batch_idx(vaccine, &app->current_date);
    printf("%u\n", batch->used);

    if (batch->used == 0) {
        name_hashmap_remove(&app->batch_name_to_vaccine_name_map, batch->name, estate);
        vaccine_remove_batch(vaccine, batch, estate);
    }

    return true;
}


void remove_all_utente_injections_and_free_utente(Application* app, Utente* utente, usize* out_removed_count, ErrorState* estate) {

    // caso não tenha sido passada uma data, remove todas as aplicações do utente
    // nesse caso, é mais eficiente apenas obter todas as aplicações do utente
    // e então deletar o utente

    usize removed_count = 0; // número de aplicações removidas
    for (usize i = 0; i < vector_size(&utente->date_injection_key_list); ++i) {
        
        bool removed_at_this_date = false; // indicador de se alguma vacina foi removida nesse dia
        Date date; // data da aplicação removida

        UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, i); // lista de aplicações do dia
        Vector* injection_key_list = &utente_injection_key_list->injection_key_list; // lista de aplicações do dia

        debug_assert(utente_injection_key_list != NULL, "utente day data is null");
        for (usize j = 0; j < vector_size(injection_key_list); ++j)  {

            char* injection_key = *(char**)vector_at(injection_key_list, j); // key da aplicação
            Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser removida
            assert(injection != NULL, "injection is null");

            if (injection->removed)
                continue;

            injection->removed = true;
            removed_at_this_date = true;
            removed_count++;
            date = injection->date;
        }

        if (removed_at_this_date)
            application_remove_removed_injections_from_list_at_date(app, &date, estate);
    }

    Utente* removed_utente = name_hashmap_remove(&app->utente_map, utente->name, estate); // utente removido do map
    utente_free(removed_utente);

    *out_removed_count = removed_count;
}

bool remove_all_injections_by_date(Application* app, Utente* utente, Date* date, usize* out_removed_count, ErrorState* estate) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&utente->date_injection_key_list, date, &found, utente_date_injection_key_list_cmp, NULL); // index resultante
    if (!found) {
        *out_removed_count = 0;
        return true;
    }

    UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // lista de aplicações do dia
    assert(utente_injection_key_list != NULL, "utente day data is null");
    Vector* injection_key_list = &utente_injection_key_list->injection_key_list; // lista de aplicações do dia
    
    usize removed_count = 0; // número de aplicações removidas
    for (usize i = 0; i < vector_size(injection_key_list); ++i) {
        
        char* injection_key = *(char**)vector_at(injection_key_list, i); // key da aplicação
        Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser removida
        assert(injection != NULL, "injection is null");

        if (injection->removed)
            continue;

        injection->removed = true;
        removed_count++;
    }
    
    // remove o dia do utente
    utente_remove_entire_day_data(utente, app, date, estate);

    // remove as aplicações do dia da lista de aplicações
    if (removed_count > 0)
        application_remove_removed_injections_from_list_at_date(app, date, estate);
    
    *out_removed_count = removed_count;
    return true;
}

bool remove_injection_by_date_and_batch(Application* app, Utente* utente, Date* date, char* batch_name, usize* out_removed_count, ErrorState* estate) {

    Injection* mock_injection = injection_create(utente->name, batch_name, *date, utente->id, estate); // aplicação falsa
    if (mock_injection == NULL)
        return false;

    char* injection_key = mock_injection->info_string; // chave da aplicação a ser removida
    if (injection_key == NULL) {
        injection_free(mock_injection);
        return false;
    }

    Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser removida
    if (injection == NULL || injection->removed) {
        injection_free(mock_injection);
        *out_removed_count = 0;
        return true;
    }

    injection->removed = true;    
    utente_remove_injection(utente, app, injection_key, estate);
    application_remove_removed_injections_from_list_at_date(app, date, estate);
    injection_free(mock_injection);
    *out_removed_count = 1;
    return true;
}

bool validate_deletar_registro_input_body(Application* app, InstructionBody_DeletarRegistro* body, ErrorState* estate) {

    // caso seja passada uma data, verifica se ela é superior à data atual
    if (body->application_date != NULL && date_cmp(body->application_date, &app->current_date) == 1) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        return false;
    }

    // caso seja passado o nome de um lote, verifica se ele existe
    if (body->batch_name != NULL && name_hashmap_find(&app->batch_name_to_vaccine_name_map, body->batch_name) == NULL) {
        set_error(ERROR_NO_SUCH_BATCH, estate, body->batch_name, NULL, NULL);
        return false;
    }

    // verifica se o nome de utente passado possuí um dono
    if (body->utente_name != NULL && name_hashmap_find(&app->utente_map, body->utente_name) == NULL) {
        set_error(ERROR_NO_SUCH_USER, estate, NULL, NULL, body->utente_name);
        return false;
    }

    return true;
}

bool handle_instruction_deletar_registro(Application* app, InstructionBody_DeletarRegistro* body, ErrorState* estate) {

    // verifica se os dados de entrada são válidos
    if (!validate_deletar_registro_input_body(app, body, estate))
        return false;
    
    // obtém o utente a partir do nome
    Utente* utente = (Utente*)name_hashmap_find(&app->utente_map, body->utente_name); // utente de quem as aplicações serão removidas
    assert(utente != NULL, "utente is null");

    // itera por todas as aplicações e procurar por aplicações que correspondam aos critérios
    usize removed_count = 0; // número de aplicações removidas
    if (body->application_date == NULL) {
        remove_all_utente_injections_and_free_utente(app, utente, &removed_count, estate);
        utente = NULL;
    }
    else if (body->batch_name == NULL) {
        if (!remove_all_injections_by_date(app, utente, body->application_date, &removed_count, estate))
            return false;
    }
    else {
        if (!remove_injection_by_date_and_batch(app, utente, body->application_date, body->batch_name, &removed_count, estate))
            return false;
    }

    // se foi removida alguma aplicação, verifica se o utente ainda possui aplicações
    // caso contrário, remove o utente
    if (removed_count > 0 && utente != NULL) {

        // verifica se o utente ainda possui aplicações
        if (utente->injection_count == 0) {
            Utente* utente = name_hashmap_remove(&app->utente_map, body->utente_name, estate); // utente removido do map
            utente_free(utente);
        }
    }

    // imprime a quantidade de aplicações removidas
    printf("%zu\n", removed_count);
    return true;
}


void print_injection_data(Injection* injection) {

    printf("%s ", injection->utente_name);
    printf("%s ", injection->batch_name);
    date_put(&injection->date);
    printf("\n");
}

bool list_utente_injections(Application* app, InstructionBody_ListarAplicações* body, ErrorState* estate) {
    
    Utente* utente = (Utente*)name_hashmap_find(&app->utente_map, body->utente_name); // utente de quem as aplicações serão listadas
    if (utente == NULL) {
        set_error(ERROR_NO_SUCH_USER, estate, NULL, NULL, body->utente_name);
        return false;
    }

    for (usize idx = 0; idx < vector_size(&utente->date_injection_key_list); ++idx) {
        
        UtenteDateInjectionList* utente_injection_key_list = (UtenteDateInjectionList*)vector_at(&utente->date_injection_key_list, idx); // lista de aplicações do dia
        assert(utente_injection_key_list != NULL, "utente day data is null");
        Vector* injection_key_list = &utente_injection_key_list->injection_key_list; // lista de aplicações do dia
        
        if (vector_size(injection_key_list) == 0)
            continue;

        for (usize j = 0; j < vector_size(injection_key_list); ++j) {
            
            char* injection_key = *(char**)vector_at(injection_key_list, j); // key da aplicação
            Injection* injection = (Injection*)name_hashmap_find(&app->injection_map, injection_key); // aplicação a ser listada
            assert(injection != NULL, "injection is null");

            print_injection_data(injection);
        }
    }
    
    return true;
}

bool handle_instruction_listar_aplicações(Application* app, InstructionBody_ListarAplicações* body, ErrorState* estate) {

    // caso seja especificado um nome de utente, busca apenas na lista desse utente
    if (body->utente_name != NULL)
        return list_utente_injections(app, body, estate);

    for (usize idx = 0; idx < name_hashmap_size(&app->injection_map); ++idx) {
        
        Injection* injection = (Injection*)name_hashmap_get_by_entry_idx(&app->injection_map, idx); // aplicação a ser listada
        if (injection == NULL)
            continue;

        // verifica se a aplicação foi removida
        if (injection->removed)
            continue;
        
        print_injection_data(injection);
    }

    return true;
}

i8 injection_key_list_date_cmp(void* a, void* b, void* ctx) {

    (void)ctx; // to prevent unused parameter error in release builds
    NameHashMapEntry* injection_entry = *(NameHashMapEntry**)a; // ponteiro convertido para o tipo correto
    Date* date = (Date*)b; // ponteiro convertido para o tipo correto

    Injection* injection = (Injection*)injection_entry->value; // aplicação a ser comparada
    return date_cmp(&injection->date, date);
}

void application_remove_removed_injections_from_list_at_date(Application* app, Date* date, ErrorState* estate) {

    bool found; // indica se a busca foi bem sucedida
    usize idx = vector_binary_search(&app->injection_map.entries_vec, date, &found, injection_key_list_date_cmp, NULL); // index resultante
    if (!found)
        return;

    // itera na direção inversa, até encontrar o primeiro elemento dessa data
    for (usize i = idx; i < name_hashmap_size(&app->injection_map); --i) {
        
        Injection* injection = (Injection*)name_hashmap_get_by_entry_idx(&app->injection_map, i); // aplicação a ser listada
        assert(injection != NULL, "injection is null");

        // verifica se a data da aplicação é diferente da data passada
        if (date_cmp(&injection->date, date) != 0)
            break;

        if (injection->removed) {
            Injection* removed_injection = name_hashmap_remove(&app->injection_map, injection->info_string, estate); // remove a aplicação do map de aplicações
            injection_free(removed_injection);
        }

        idx = i;
    }

    // itera na direção normal, passando por todos os elementos restantes da mesma data
    for (usize i = idx; i < name_hashmap_size(&app->injection_map); ++i) {
        
        Injection* injection = (Injection*)name_hashmap_get_by_entry_idx(&app->injection_map, i); // aplicação a ser listada
        assert(injection != NULL, "injection is null");

        // verifica se a data da aplicação é diferente da data passada
        if (date_cmp(&injection->date, date) != 0)
            break;

        if (!injection->removed)
            continue;

        // remove a aplicação da lista de aplicações
        Injection* removed_injection = name_hashmap_remove(&app->injection_map, injection->info_string, estate); // remove a aplicação do map de aplicações
        injection_free(removed_injection);
        i--;
    }
}


bool handle_instruction_avançar_tempo(Application* app, InstructionBody_AvançarTempo* body, ErrorState* estate) {

    // caso tenha uma data como argumento, o comando deve alterar a data atual da aplicação para essa
    // nesse caso, a saída será a nova data
    if (body->new_date != NULL) {

        // verifica se a nova data é superior à data atual
        if (date_cmp(&app->current_date, body->new_date) == 1) {
            set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
            return false;
        }

        // atualiza a data atual e imprime a nova data
        app->current_date = *body->new_date;
        date_put(&app->current_date); printf("\n");

    } else {
        
        // caso contrário, a saída será a data atual
        date_put(&app->current_date); printf("\n");
    }

    // atualiza o index de último lote válido de todas as vacinas
    for (usize i = 0; i < name_hashmap_size(&app->vaccine_map); ++i) {

        Vaccine* vaccine = (Vaccine*)name_hashmap_get_by_entry_idx(&app->vaccine_map, i); // vacina a ser atualizada
        assert(vaccine != NULL, "vaccine is null");

        vaccine_update_oldest_valid_batch_idx(vaccine, &app->current_date);
    }

    return true;
}


#define HANDLE_INSTRUCTION_SWITCH_CASE(up_case, low_case) \
    case INSTRUCTION_TYPE_##up_case: { \
        result = handle_instruction_##low_case(app, instr->body, estate); \
        instruction_body_##low_case##_free(instr->body); \
        break; \
    }

bool handle_instruction(Application* app, Instruction* instr, bool* exit, ErrorState* estate) {

    bool result = false; // variável que indica se a instrução foi executada com sucesso
    switch (instr->type) {
        
        case INSTRUCTION_TYPE_TERMINAR:
            *exit = true;
            result = true;
            instruction_body_terminar_free(instr->body);
            break;

        HANDLE_INSTRUCTION_SWITCH_CASE(CRIAR_LOTE, criar_lote)
        HANDLE_INSTRUCTION_SWITCH_CASE(LISTAR_VACINAS, listar_vacinas)
        HANDLE_INSTRUCTION_SWITCH_CASE(APLICAR_DOSE, aplicar_dose)
        HANDLE_INSTRUCTION_SWITCH_CASE(RETIRAR_LOTE, retirar_lote)
        HANDLE_INSTRUCTION_SWITCH_CASE(DELETAR_REGISTRO, deletar_registro)
        HANDLE_INSTRUCTION_SWITCH_CASE(LISTAR_APLICAÇÕES, listar_aplicações)
        HANDLE_INSTRUCTION_SWITCH_CASE(AVANÇAR_TEMPO, avançar_tempo)
    }

    return result;
}
