/**
 * @file data_types.h
 * @author ist1106900 (Goncalo Aleixo)
*/

#pragma once

/// \brief Estruturas de dados e funções auxiliares para manipulação de dados

// local
#include "core.h"



/* -------------------------------------------------------------------------- */
/*                                    date                                    */
/* -------------------------------------------------------------------------- */

/// \class Date
/// \brief Estrutura que representa uma data, com dia, mês e ano
typedef struct {
    u8 day; //!< Dia do mês (1-31)
    u8 month; //!< Mês do ano (1-12)
    u32 year; //!< Ano (1 ou mais dígitos)
} Date;

/// \public \memberof Date
/// \brief Verifica se a data é válida.
/// \param date Ponteiro para a data a ser verificada.
/// \return true se a data é válida, false caso contrário.
bool date_is_valid(Date const* date);

/// \public \memberof Date
/// \brief Função de comparação de datas, para ordenação.
/// \param lhs Ponteiro para a data à esquerda.
/// \param rhs Ponteiro para a data à direita.
/// \return -1 se lhs < rhs, 0 se lhs == rhs, 1 se lhs > rhs.
i8 date_cmp(Date const* lhs, Date const* rhs);

/// \public \memberof Date
/// \brief Escreve a data no formato "dd/mm/aaaa" no stdout.
/// \param date Ponteiro para a data a ser escrita.
void date_put(Date const* date);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Date
/// \brief Verifica se o ano é bissexto.
/// \param year Ano a ser verificado.
/// \return true se o ano é bissexto, false caso contrário.
bool is_leap_year(u32 year);



/* -------------------------------------------------------------------------- */
/*                                   vector                                   */
/* -------------------------------------------------------------------------- */

/// \class Vector
/// \brief Estrutura que representa um vetor dinâmico de dados.
typedef struct {
    usize size; //!< Número de elementos no vetor
    usize capacity; //!< Capacidade do vetor
    usize element_size; //!< Tamanho de cada elemento no vetor
    void(*free_fn)(void*); //!< Função de liberação de memória para os elementos do vetor
    void* data; //!< Ponteiro para os dados do vetor, de ownership próprio
} Vector;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Vector
/// \brief Inicializa um vetor dinâmico.
/// \param vec Ponteiro para o vetor a ser inicializado.
/// \param element_size Tamanho de cada elemento no vetor.
/// \param free_fn Função de liberação de memória para os elementos do vetor.
///     \note Se NULL, não será chamada.
void vector_init(Vector* vec, usize element_size, void(*free_fn)(void*));

/// \public \memberof Vector
/// \brief Libera a memória do vetor.
/// \param vec Ponteiro para o vetor a ser liberado.
///     \note A memória onde o vetor foi alocada não é de ownership próprio,
///           logo, não será liberada.
/// \note Caso a função de liberação de memória seja NULL, não será chamada.
void vector_free(void* vec);

/* --------------------------------- métodos -------------------------------- */

/// \public \memberof Vector
/// \brief Obtém o tamanho do vetor.
/// \param vec Ponteiro para o vetor.
/// \return O número de elementos no vetor.
usize vector_size(Vector const* vec);

/// \public \memberof Vector
/// \brief Obtém o elemento no índice especificado.
/// \param vec Ponteiro para o vetor.
/// \param idx Índice do elemento a ser obtido.
/// \return Ponteiro para o elemento no índice especificado.
///     \note O valor retornado é de ownership do vetor.
///     \note O valor retornado não deve ser liberado.
///     \note O valor retornado pode ser NULL, caso o índice seja inválido.
void* vector_at(Vector* vec, usize idx);

/// \public \memberof Vector
/// \brief Inserte um elemento no vetor.
/// \param vec Ponteiro para o vetor.
/// \param idx Índice onde o elemento será inserido.
/// \param data Ponteiro para o elemento a ser inserido.
///     \note O valor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a inserção foi bem-sucedida, false caso contrário.
bool vector_insert(Vector* vec, usize idx, void* data, ErrorState* estate);

/// \public \memberof Vector
/// \brief Remove um elemento do vetor.
/// \param vec Ponteiro para o vetor.
/// \param idx Índice do elemento a ser removido.
/// \param out_value_dst Ponteiro para onde o valor removido será armazenado.
///     \note O valor é de ownership do chamador.
///     \note O valor pode ser NULL, neste caso, o valor será liberado pelo vetor.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a remoção foi bem-sucedida, false caso contrário.
bool vector_remove(Vector* vec, usize idx, void* out_value_dst, ErrorState* estate);

/// \public \memberof Vector
/// \brief Efetua uma busca binária no vetor.
/// \param vec Ponteiro para o vetor.
/// \param data Ponteiro para o elemento a ser buscado.
///     \note O valor é de ownership do chamador.
/// \param out_found Ponteiro para onde o resultado da busca será armazenado.
/// \param cmp_fn Função de comparação.
///     \note A função deve retornar -1 se o primeiro elemento for menor que o segundo,
///           0 se forem iguais e 1 se o primeiro elemento for maior que o segundo.
/// \param cmp_fn_ctx Ponteiro para o contexto da função de comparação.
///     \note O valor é de ownership do chamador.
usize vector_binary_search(Vector* vec, void* data, bool* out_found, i8(*cmp_fn)(void*, void*, void*), void* cmp_fn_ctx);

/// \public \memberof Vector
/// \brief Redimensiona o vetor.
/// \param vec Ponteiro para o vetor.
/// \param new_size Novo tamanho do vetor.
/// \param init_data Ponteiro para os dados que serão inseridos em posições novas.
///     \note O valor será copiado em cada nova posição.
///     \note Se NULL, não será copiado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o redimensionamento foi bem-sucedido, false caso contrário.
bool vector_resize(Vector* vec, usize new_size, void* init_data, ErrorState* estate);

/// \public \memberof Vector
/// \brief Remove todos os elementos do vetor.
/// \param vec Ponteiro para o vetor.
/// \note Todos os elementos serão liberados.
void vector_clear(Vector* vec);

/* ------------------------------- utilitários ------------------------------ */

/// \public \memberof Vector
/// \brief Libera a memória de um vetor de strings.
/// \param ptr Ponteiro para o vetor de strings.
void vector_string_free(void* ptr);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Vector
/// \brief Aumenta o tamanho do vetor.
/// \param vec Ponteiro para o vetor.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o aumento foi bem-sucedido, false caso contrário.
bool vector_grow(Vector* vec, ErrorState* estate);

/// \private \memberof Vector
/// \brief Redimensiona o vetor.
/// \param vec Ponteiro para o vetor.
/// \param new_size Novo tamanho do vetor.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o redimensionamento foi bem-sucedido, false caso contrário.
bool vector_realloc(Vector* vec, usize new_size, ErrorState* estate);



/* -------------------------------------------------------------------------- */
/*                                name_hashmap                                */
/* -------------------------------------------------------------------------- */

/// \class NameHashMapEntry
/// \brief Estrutura que representa uma entrada no hashmap.
typedef struct {
    char const* key; //!< Chave da entrada, podem ser de ownership próprio
    void* value; //!< Valor da entrada, de ownership próprio
    u32 before_idx; //!< Índice da entrada anterior na lista encadeada
    u32 after_idx; //!< Índice da entrada seguinte na lista encadeada
} NameHashMapEntry;

/// \class NameHashMap
/// \brief Hashmap de strings para ponteiros.
/// \details O hashmap é implementado usando buckets, onde cada bucket pode possuir
///          uma lista encadeada de entradas.
typedef struct {
    Vector entries_vec; //!< Vetor de entradas alocadas no hashmap
    Vector buckets_vec; //!< Vetor de buckets do hashmap
    void(*free_fn)(void*); //!< Função de liberação de memória para os valores do hashmap
    bool own_keys; //!< Indica se as chaves são de ownership próprio
} NameHashMap;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof NameHashMap
/// \brief Inicializa um hashmap.
/// \param map Ponteiro para o hashmap a ser inicializado.
/// \param free_fn Função de liberação de memória para os valores do hashmap.
///     \note Se NULL, não será chamada.
/// \param own_keys Indica se as chaves são de ownership próprio.
///     \note Se true, as chaves serão liberadas quando o hashmap for liberado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a inicialização foi bem-sucedida, false caso contrário.
bool name_hashmap_init(NameHashMap* map, void(*free_fn)(void*), bool own_keys, ErrorState* estate);

/// \public \memberof NameHashMap
/// \brief Libera a memória do hashmap.
/// \param map Ponteiro para o hashmap a ser liberado.
///     \note A memória onde o hashmap foi alocada não é de ownership próprio,
///           logo, não será liberada.
void name_hashmap_free(void* map);


/* --------------------------------- métodos -------------------------------- */

/// \public \memberof NameHashMapEntry
/// \brief Cria uma nova entrada para o hashmap.
/// \param key Chave da entrada.
///     \note O valor pode ser tomado, de acordo com o parâmetro 'copy_key'.
/// \param value Valor da entrada.
///     \note O valor é de ownership próprio.
/// \param copy_key Indica se a chave deve ser copiada.
///     \note Se true, a chave será copiada.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para a nova entrada.
///     \note O valor retornado é de ownership do chamador.
///     \note Retorna NULL em caso de erro.
NameHashMapEntry* name_hashmap_entry_new(char const* key, void* value, bool copy_key, ErrorState* estate);

/// \public \memberof NameHashMapEntry
/// \brief Libera a memória da entrada do hashmap.
/// \param entry Ponteiro para a entrada a ser liberada.
/// \param map Ponteiro para o hashmap.
void name_hashmap_entry_free(NameHashMapEntry* entry, NameHashMap* map);

/// \public \memberof NameHashMap
/// \brief Insere uma nova entrada no hashmap.
/// \param map Ponteiro para o hashmap.
/// \param key Chave da entrada.
///     \note O valor pode ser tomado, de acordo com o parâmetro 'own_keys' do construtor.
/// \param value Valor da entrada.
///     \note O valor é de ownership próprio.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a inserção foi bem-sucedida, false caso contrário.
bool name_hashmap_insert(NameHashMap* map, char const* key, void* value, ErrorState* estate);

/// \public \memberof NameHashMap
/// \brief Remove uma entrada do hashmap.
/// \param map Ponteiro para o hashmap.
/// \param key Chave da entrada a ser removida.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o valor removido.
///     \note O valor é de ownership do chamador.
///     \note O valor pode ser NULL, caso a chave não exista.
void* name_hashmap_remove(NameHashMap* map, char const* key, ErrorState* estate);

/// \public \memberof NameHashMap
/// \brief Busca uma entrada no hashmap.
/// \param map Ponteiro para o hashmap.
/// \param key Chave da entrada a ser buscada.
/// \return Ponteiro para o valor da entrada.
///     \note O valor é de ownership do hashmap.
///     \note O valor pode ser NULL, caso a chave não exista.
void* name_hashmap_find(NameHashMap* map, char const* key);

/// \public \memberof NameHashMap
/// \brief Verifica se uma chave existe no hashmap.
/// \param map Ponteiro para o hashmap.
/// \param key Chave a ser verificada.
/// \return true se a chave existe, false caso contrário.
bool name_hashmap_contains(NameHashMap* map, char const* key);

/// \public \memberof NameHashMap
/// \brief Obtém o número de entradas no hashmap.
/// \param map Ponteiro para o hashmap.
/// \return O número de entradas no hashmap.
usize name_hashmap_size(NameHashMap const* map);

/// \public \memberof NameHashMap
/// \brief Obtém o valor de uma entrada pelo índice dela no vetor de entradas.
/// \param map Ponteiro para o hashmap.
/// \param entry_idx Índice da entrada no vetor de entradas.
/// \return Ponteiro para o valor da entrada.
///     \note O valor é de ownership do hashmap.
///     \note O valor pode ser NULL, caso o índice seja inválido.
void* name_hashmap_get_by_entry_idx(NameHashMap* map, usize entry_idx);

/// \public \memberof NameHashMap
/// \brief Obtém a chave de uma entrada pelo índice dela no vetor de entradas.
/// \param map Ponteiro para o hashmap.
/// \param entry_idx Índice da entrada no vetor de entradas.
/// \return Ponteiro para a chave da entrada.
///     \note O valor é de ownership do hashmap.
///     \note O valor pode ser NULL, caso o índice seja inválido.
char* name_hashmap_get_key_by_entry_idx(NameHashMap* map, usize entry_idx);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof NameHashMap
/// \brief Verifica se o hashmap tem espaço suficiente para ser redimensionado.
///        Caso verdadeiro, redimensiona o hashmap.
/// \param map Ponteiro para o hashmap.
void name_hashmap_shrink_if_possible(NameHashMap* map);

/// \private \memberof NameHashMap
/// \brief Atualiza os indices das entradas do hashmap de acordo com a remoção de um valor.
/// \param map Ponteiro para o hashmap.
/// \param entry Ponteiro para a entrada que foi removida.
void handle_removed_entry(NameHashMap* map, NameHashMapEntry* entry);

/// \private \memberof NameHashMap
/// \brief Redimensiona o hashmap caso necessário.
/// \param map Ponteiro para o hashmap.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o redimensionamento foi bem-sucedido, false caso contrário.
bool name_hashmap_grow(NameHashMap* map, ErrorState* estate);

/// \private \memberof NameHashMap
/// \brief Faz o rehash de todas as entradas do hashmap.
/// \param map Ponteiro para o hashmap.
void name_hashmap_rehash(NameHashMap* map);

/// \private \memberof NameHashMap
/// \brief Inicializa o vetor de buckets do hashmap.
/// \param map Ponteiro para o hashmap.
/// \param size Tamanho do vetor de buckets.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a inicialização foi bem-sucedida, false caso contrário.
bool name_hashmap_init_bucket_vec(NameHashMap* map, usize size, ErrorState* estate);

/// \private \memberof NameHashMap
/// \brief Atualiza os índices das entradas vizinhas de uma entrada.
/// \param map Ponteiro para o hashmap.
/// \param entry Ponteiro para a entrada.
/// \param entry_new_idx Novo índice da entrada.
void name_hashmap_update_indexes_for_entry(NameHashMap* map, NameHashMapEntry* entry, usize entry_new_idx);

/// \private \memberof NameHashMap
/// \brief Insere uma nova entrada no vetor de entradas do hashmap, ao lado de uma entrada existente.
/// \param map Ponteiro para o hashmap.
/// \param entry_idx Índice da nova entrada.
/// \param bucket_idx Índice do bucket onde a nova entrada será inserida.
/// \param bucket_entry_idx Índice da entrada existente ao lado da nova entrada.
void name_hashmap_insert_entry_at_side_of_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_idx, usize bucket_entry_idx);

/// \private \memberof NameHashMap
/// \brief Insere uma nova entrada no vetor de entradas do hashmap, após uma entrada existente.
/// \param map Ponteiro para o hashmap.
/// \param entry_idx Índice da nova entrada.
/// \param bucket_entry_idx Índice da entrada existente após a nova entrada.
void name_hashmap_insert_entry_after_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_entry_idx);

/// \private \memberof NameHashMap
/// \brief Insere uma nova entrada no vetor de entradas do hashmap, antes de uma entrada existente.
/// \param map Ponteiro para o hashmap.
/// \param entry_idx Índice da nova entrada.
/// \param bucket_entry_idx Índice da entrada existente antes da nova entrada.
void name_hashmap_insert_entry_before_bucket_entry(NameHashMap* map, usize entry_idx, usize bucket_entry_idx);

/// \private \memberof NameHashMap
/// \brief Verifica se uma entrada existe num bucket do hashmap.
/// \param map Ponteiro para o hashmap.
/// \param key Chave da entrada.
/// \param bucket_idx Índice do bucket onde a entrada deve ser buscada.
/// \param[out] out_idx Ponteiro para onde o índice da entrada será armazenado.
/// \return true se a entrada existe, false caso contrário.
bool name_hashmap_find_key_entry_bucket_idx(NameHashMap* map, const char* key, usize bucket_idx, usize* out_idx);

/// \private \memberof NameHashMap
/// \brief Calcula o índice do bucket onde uma entrada deve ser inserida.
/// \param map Ponteiro para o hashmap.
/// \param key Chave da entrada.
/// \return Índice do bucket onde a entrada deve ser inserida.
usize name_hashmap_calc_bucket(NameHashMap* map, const char* key);

/// \private \memberof NameHashMap
/// \brief Faz o hash da chave.
/// \param key Chave a ser hasheada.
/// \return O hash da chave.
usize name_hashmap_hash(const char* key);
