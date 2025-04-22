/**
 * @file application.h
 * @author ist1106900 (Goncalo Aleixo)
*/

#pragma once

/// \brief Cabeçalho do módulo de aplicação, contendo as definições de tipos e funções
///        para manipulação de vacinas, lotes, aplicações e processamento de instruções.

// local
#include "core.h"
#include "data_types.h"
#include "parsers.h"


/* -------------------------------------------------------------------------- */
/*                                    batch                                   */
/* -------------------------------------------------------------------------- */

/// \class Batch
/// \brief Estrutura que representa um lote de vacina.
typedef struct {
    char* name; //!< Nome do lote, o ownership é do lote
    char* vaccine_name_ref; //!< Nome da vacina a qual o lote pertence, o ownership NÃO é do lote
    Date expiration_date; //!< Data de expiração do lote
    u32 quantity; //!< Quantidade total de doses do lote
    u32 used; //!< Quantidade de doses já aplicadas 
    bool retired; //!< Indica se o lote foi retirado
} Batch;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Batch
/// \brief Cria um novo lote de vacina.
/// \param name_ref Nome do lote.
///     \note O ownership do nome continua sendo do chamador
/// \param vaccine_name_ref Nome da vacina.
///     \note O ownership do nome continua sendo do chamador
/// \param expiration_date Data de expiração do lote.
/// \param quantity Quantidade total de doses do lote.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o lote criado, ou NULL em caso de erro.
Batch* batch_create(char* name_ref, char* vaccine_name_ref, Date expiration_date, u32 quantity, ErrorState* estate);

/// \public \memberof Batch
/// \brief Libera a memória alocada para um lote.
/// \param _batch Ponteiro para o lote a ser liberado.
void batch_free(void* _batch);

/* --------------------------------- métodos -------------------------------- */

/// \public \memberof Batch
/// \brief Compara dois lotes, para fins de ordenação.
/// \param a Ponteiro para o primeiro lote.
/// \param b Ponteiro para o segundo lote.
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 batch_cmp(Batch* a, Batch* b);

/// \public \memberof Batch
/// \brief Encontra o índice em um vetor de lotes onde um novo lote deve ser inserido.
/// \param batch_list Ponteiro para o vetor de lotes.
/// \param batch Ponteiro para o lote a ser inserido.
/// \return O índice onde o lote deve ser inserido.
usize find_new_batch_list_idx(Vector* batch_list, Batch* batch);

/// \public \memberof Batch
/// \brief Encontra o índice de um lote em um vetor de lotes.
/// \param batch_list Ponteiro para o vetor de lotes.
/// \param batch Ponteiro para o lote a ser encontrado.
/// \return O índice do lote encontrado.
usize find_batch_list_idx(Vector* batch_list, Batch* batch);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Batch
/// \brief Compara dois ponteiros para lotes.
/// \param _a Ponteiro para o primeiro lote.
/// \param _b Ponteiro para o segundo lote.
/// \param ctx Ponteiro para o contexto (não utilizado).
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 batch_cmp_ptr(void* _a, void* _b, void* ctx);



/* -------------------------------------------------------------------------- */
/*                                   vaccine                                  */
/* -------------------------------------------------------------------------- */

/// \class Vaccine
/// \brief Estrutura que representa uma vacina.
typedef struct {
    char* name; //!< Nome da vacina, o ownership é da vacina
    NameHashMap batches; //!< Map de lotes
    Vector batch_list; //!< Lista ordenada de lotes
    usize oldest_valid_batch_idx; //!< Índice do lote mais antigo válido
} Vaccine;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Vaccine
/// \brief Cria uma nova vacina.
/// \param name Nome da vacina.
///     \note O ownership do nome continua sendo do chamador
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para a vacina criada, ou NULL em caso de erro.
Vaccine* vaccine_create(char* name, ErrorState* estate);

/// \public \memberof Vaccine
/// \brief Libera a memória alocada para uma vacina.
/// \param _vaccine Ponteiro para a vacina a ser liberada.
void vaccine_free(void* _vaccine);

/* --------------------------------- métodos -------------------------------- */

/// \public \memberof Vaccine
/// \brief Adiciona um lote a uma vacina.
/// \param vaccine Ponteiro para a vacina.
/// \param batch Ponteiro para o lote a ser adicionado.
/// \param current_date Data atual.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o lote foi adicionado com sucesso, false em caso de erro.
bool vaccine_add_batch(Vaccine* vaccine, Batch* batch, Date* current_date, ErrorState* estate);

/// \public \memberof Vaccine
/// \brief Remove um lote de uma vacina.
/// \param vaccine Ponteiro para a vacina.
/// \param batch Ponteiro para o lote a ser removido.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o lote foi removido com sucesso, false em caso de erro.
bool vaccine_remove_batch(Vaccine* vaccine, Batch* batch, ErrorState* estate);

/// \public \memberof Vaccine
/// \brief Atualiza o índice do lote mais antigo válido.
/// \param vaccine Ponteiro para a vacina.
/// \param current_date Data atual.
void vaccine_update_oldest_valid_batch_idx(Vaccine* vaccine, Date* current_date);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Vaccine
/// \brief Atualiza o índice do lote mais antigo válido.
/// \param vaccine Ponteiro para a vacina.
/// \param current_date Data atual.
void update_oldest_valid_batch_idx(Vaccine* vaccine, Date* current_date);

/// \private \memberof Vaccine
/// \brief Encontra o lote mais antigo válido.
/// \param vaccine Ponteiro para a vacina.
/// \param current_date Data atual.
void find_oldest_valid_batch(Vaccine* vaccine, Date* current_date);



/* -------------------------------------------------------------------------- */
/*                                  injection                                 */
/* -------------------------------------------------------------------------- */

/// \class Injection
/// \brief Estrutura que representa uma aplicação de vacina em um utente.
typedef struct {
    char* utente_name; //!< Nome do utente, o ownership NÃO é da aplicação
    char* batch_name; //!< Nome do lote, o ownership NÃO é da aplicação
    char* info_string; //!< String de informação da aplicação, o ownership é da aplicação
    Date date; //!< Data da aplicação
    bool removed; //!< Indica se a aplicação foi removida
} Injection;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Injection
/// \brief Cria uma nova aplicação.
/// \param utente_name Nome do utente.
///     \note O ownership do nome continua sendo do chamador
/// \param batch_name Nome do lote.
///     \note O ownership do nome continua sendo do chamador
/// \param date Data da aplicação.
/// \param utente_id ID do utente.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para a aplicação criada, ou NULL em caso de erro.
Injection* injection_create(char* utente_name, char* batch_name, Date date, u64 utente_id, ErrorState* estate);

/// \public \memberof Injection
/// \brief Libera a memória alocada para uma aplicação.
/// \param _injection Ponteiro para a aplicação a ser liberada.
void injection_free(void* _injection);

/* --------------------------------- métodos -------------------------------- */

/// \private \memberof Injection
/// \brief Retorna a chave de uma aplicação.
/// \param injection Ponteiro para a aplicação.
/// \param utente_id ID do utente.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para a chave da aplicação, ou NULL em caso de erro.
char* injection_get_info_string(Injection* injection, u64 utente_id, ErrorState* estate);

/// \public \memberof Injection
/// \brief Compara duas aplicações, para fins de ordenação.
/// \param a Ponteiro para a primeira aplicação.
/// \param b Ponteiro para a segunda aplicação.
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 injection_cmp(Injection* a, Injection* b);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Injection
/// \brief Determina o tamanho da string de um número.
/// \param num Número a ser convertido.
/// \return Tamanho da string do número.
usize usize_decimal_length(usize num);



/* -------------------------------------------------------------------------- */
/*                                 application                                */
/* -------------------------------------------------------------------------- */

/// \class Utente
/// \brief Estrutura que representa um utente (paciente) que recebeu vacinas.
typedef struct {
    char* name; //!< Nome do utente, o ownership é do utente
    u64 id; //!< ID do utente
    usize injection_count; //!< Contador de injeções aplicadas
    Vector date_injection_key_list; //!< Lista de listas de injeções aplicadas, ordenada por data, listas internas ordenadas por ordem de inserção.
    Vector vaccine_last_injection_key_list; //!< Lista de vacinas aplicadas, ordenada por nome da vacina, cada elemento contém o nome da vacina e a chave da última injeção aplicada.
} Utente;

/// \class Application
/// \brief Estrutura que representa a aplicação do sistema.
typedef struct {
    NameHashMap injection_map; //!< Map de aplicações, onde a chave é a string de informação da aplicação
    NameHashMap vaccine_map; //!< Map de vacinas, onde a chave é o nome da vacina
    NameHashMap batch_name_to_vaccine_name_map; //!< Map de lotes, onde a chave é o nome do lote e o valor é o nome da vacina
    NameHashMap utente_map; //!< Map de utentes, onde a chave é o nome do utente
    u64 utente_id_counter; //!< Contador de IDs de utentes
    Date current_date; //!< Data atual
} Application;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Application
/// \brief Inicializa a aplicação.
/// \param app Ponteiro para a aplicação.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a aplicação foi inicializada com sucesso, false em caso de erro.
bool application_init(Application* app, ErrorState* estate);

/// \public \memberof Application
/// \brief Libera a memória alocada para a aplicação.
/// \param app Ponteiro para a aplicação.
void application_free(Application* app);

/* --------------------------------- métodos -------------------------------- */

/// \public \memberof Application
/// \brief Processa uma instrução.
/// \param app Ponteiro para a aplicação.
/// \param instr Ponteiro para a instrução.
/// \param exit Ponteiro para um booleano que indica se a aplicação deve ser encerrada.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction(Application* app, Instruction* instr, bool* exit, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de criar lote.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_criar_lote(Application* app, InstructionBody_CriarLote* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de listar vacinas.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_listar_vacinas(Application* app, InstructionBody_ListarVacinas* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de aplicar dose.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_aplicar_dose(Application* app, InstructionBody_AplicarDose* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de retirar lote.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_retirar_lote(Application* app, InstructionBody_RetirarLote* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de deletar registro.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_deletar_registro(Application* app, InstructionBody_DeletarRegistro* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de listar aplicações.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_listar_aplicações(Application* app, InstructionBody_ListarAplicações* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Processa uma instrução de avançar tempo.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a instrução foi processada com sucesso, false em caso de erro.
bool handle_instruction_avançar_tempo(Application* app, InstructionBody_AvançarTempo* body, ErrorState* estate);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Application
/// \brief Cria uma nova vacina e a adiciona ao mapa de vacinas.
/// \param app Ponteiro para a aplicação.
/// \param vaccine_name Nome da vacina.
///     \note O ownership do nome continua sendo do chamador
/// \param estate Ponteiro para o estado de erro.
/// \return true se a vacina foi criada com sucesso, false em caso de erro.
bool new_vaccine(Application* app, char* vaccine_name, ErrorState* estate);

/// \private \memberof Application
/// \brief Imprime os dados de um lote.
/// \param batch Ponteiro para o lote.
void application_show_batch(Batch* batch);

/// \private \memberof Application
/// \brief Obtém uma vacina a partir do nome.
/// \param app Ponteiro para a aplicação.
/// \param vaccine_name Nome da vacina.
///     \note O ownership do nome continua sendo do chamador
/// \param dst_vaccine[out] Ponteiro para aonde a o ponteiro da vacina será armazenado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a vacina foi encontrada, false em caso de erro.
bool get_vaccine_by_name(Application* app, char* vaccine_name, Vaccine** dst_vaccine, ErrorState* estate);

/// \private \memberof Application
/// \brief Lista todas as vacinas aplicadas em um utente.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a lista foi criada com sucesso, false em caso de erro.
bool handle_instruction_listar_vacinas_utente(Application* app, InstructionBody_ListarVacinas* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Verifica se os dados de entrada para a instrução de aplicar dose são válidos.
///        Obtém ou cria a vacina e o lote se necessário.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param out_vaccine[out] Ponteiro para a vacina.
/// \param out_batch[out] Ponteiro para o lote.
/// \param estate Ponteiro para o estado de erro.
/// \return true se os dados de entrada são válidos, false em caso de erro.
bool handle_instruction_aplicar_dose_preparer(Application* app, InstructionBody_AplicarDose* body, Vaccine** out_vaccine, Batch** out_batch, ErrorState* estate);

/// \private \memberof Application
/// \brief Verifica se o utente já foi vacinado com a vacina nesse mesmo dia.
/// \param app Ponteiro para a aplicação.
/// \param utente Ponteiro para o utente.
/// \param vaccine_name Nome da vacina.
///     \note O ownership do nome continua sendo do chamador
/// \param estate Ponteiro para o estado de erro.
/// \return true se o utente não foi vacinado com a vacina nesse dia, false em caso de erro.
bool check_utente_injection_compatibility(Application* app, Utente* utente, char* vaccine_name, ErrorState* estate);

/// \private \memberof Application
/// \brief Remove todas as injeções de um utente e libera o utente.
/// \param app Ponteiro para a aplicação.
/// \param utente Ponteiro para o utente.
/// \param out_removed_count[out] Ponteiro para o contador de injeções removidas.
/// \param estate Ponteiro para o estado de erro.
/// \return true se as injeções foram removidas com sucesso, false em caso de erro.
void remove_all_utente_injections_and_free_utente(Application* app, Utente* utente, usize* out_removed_count, ErrorState* estate);

/// \private \memberof Application
/// \brief Remove todas as injeções de um utente em uma data específica.
/// \param app Ponteiro para a aplicação.
/// \param utente Ponteiro para o utente.
/// \param date Ponteiro para a data.
/// \param out_removed_count[out] Ponteiro para o contador de injeções removidas.
/// \param estate Ponteiro para o estado de erro.
/// \return true se as injeções foram removidas com sucesso, false em caso de erro.
bool remove_all_injections_by_date(Application* app, Utente* utente, Date* date, usize* out_removed_count, ErrorState* estate);

/// \private \memberof Application
/// \brief Remove uma injeção de um utente em uma data específica com um nome de lote específico.
/// \param app Ponteiro para a aplicação.
/// \param utente Ponteiro para o utente.
/// \param date Ponteiro para a data.
/// \param batch_name Nome do lote.
///     \note O ownership do nome continua sendo do chamador
/// \param out_removed_count[out] Ponteiro para o contador de injeções removidas.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi removida com sucesso, false em caso de erro.
bool remove_injection_by_date_and_batch(Application* app, Utente* utente, Date* date, char* batch_name, usize* out_removed_count, ErrorState* estate);

/// \private \memberof Application
/// \brief Verifica se os dados de entrada para a instrução de deletar registro são válidos.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se os dados de entrada são válidos, false em caso de erro.
bool validate_deletar_registro_input_body(Application* app, InstructionBody_DeletarRegistro* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Imprime os dados de uma injeção.
/// \param injection Ponteiro para a injeção.
void print_injection_data(Injection* injection);

/// \private \memberof Application
/// \brief Lista todas as injeções aplicadas a um utente.
/// \param app Ponteiro para a aplicação.
/// \param body Ponteiro para o corpo da instrução.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a lista foi criada com sucesso, false em caso de erro.
bool list_utente_injections(Application* app, InstructionBody_ListarAplicações* body, ErrorState* estate);

/// \private \memberof Application
/// \brief Compara duas injeções numa lista ordenada de injeções.
/// \param a Ponteiro para a primeira injeção.
/// \param b Ponteiro para a segunda injeção.
/// \param ctx Ponteiro para o contexto (não utilizado).
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 injection_key_list_date_cmp(void* a, void* b, void* ctx);

/// \private \memberof Application
/// \brief Remove injeções removidas do mapa de injeções.
/// \param app Ponteiro para a aplicação.
/// \param date Ponteiro para a data.
/// \param estate Ponteiro para o estado de erro.
/// \return true se as injeções removidas foram removidas com sucesso, false em caso de erro.
void application_remove_removed_injections_from_list_at_date(Application* app, Date* date, ErrorState* estate);



/* -------------------------------------------------------------------------- */
/*                                   utente                                   */
/* -------------------------------------------------------------------------- */

/// \class UtenteDateInjectionList
/// \brief Estrutura que representa uma lista de injeções aplicadas em um utente em uma data específica.
typedef struct {
    Date date; //!< Data da aplicação
    Vector injection_key_list; //!< Lista de chaves de injeções aplicadas
} UtenteDateInjectionList;

/// \class VaccineNameLastInjectionKey
/// \brief Estrutura que representa uma vacina e a chave da última injeção aplicada.
typedef struct {
    char* vaccine_name; //!< Nome da vacina, o ownership NÃO é da estrutura
    char* injection_key; //!< Chave da última injeção aplicada, o ownership NÃO é da estrutura
} VaccineNameLastInjectionKey;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof Utente
/// \brief Cria um novo utente.
/// \param name Nome do utente.
///     \note O ownership do nome é do utente
/// \param id ID do utente.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o utente criado, ou NULL em caso de erro.
Utente* utente_create(char* name, u64 id, ErrorState* estate);

/// \public \memberof Utente
/// \brief Libera a memória alocada para um utente.
/// \param _utente Ponteiro para o utente a ser liberado.
void utente_free(void* _utente);

/* --------------------------------- métodos -------------------------------- */

/// \public \memberof Utente
/// \brief Adiciona uma injeção a um utente.
/// \param utente Ponteiro para o utente.
/// \param app Ponteiro para a aplicação.
/// \param injection_key Ponteiro para a chave da injeção.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi adicionada com sucesso, false em caso de erro.
bool utente_add_injection(Utente* utente, Application* app, char* injection_key, ErrorState* estate);

/// \public \memberof Utente
/// \brief Remove uma injeção de um utente.
/// \param utente Ponteiro para o utente.
/// \param app Ponteiro para a aplicação.
/// \param injection_key Ponteiro para a chave da injeção.
/// \param estate Ponteiro para o estado de erro.
void utente_remove_injection(Utente* utente, Application* app, char* injection_key, ErrorState* estate);

/* -------------------------------- internals ------------------------------- */

/// \private \memberof Utente
/// \brief Função de comparação para encontrar uma lista de injeções aplicadas em uma data específica.
/// \param _a Ponteiro para a lista de listas de injeções.
/// \param _b Ponteiro para a data.
/// \param ctx Ponteiro para o contexto (não utilizado).
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 utente_date_injection_key_list_cmp(void* _a, void* _b, void* ctx);

/// \private \memberof Utente
/// \brief Função de comparação para encontrar a posição de uma vacina na lista de vacinas aplicadas a partir do nome.
/// \param _a Ponteiro para a lista de vacinas.
/// \param _b Ponteiro para o nome da vacina.
/// \param ctx Ponteiro para o contexto (não utilizado).
/// \return Um valor negativo se a < b, zero se a == b e um valor positivo se a > b.
i8 utente_vaccine_last_injection_key_list_cmp(void* _a, void* _b, void* ctx);

/// \private \memberof Utente
/// \brief Adiciona nova injeção a lista de vacinas aplicadas.
/// \param utente Ponteiro para o utente.
/// \param app Ponteiro para a aplicação.
/// \param injection Ponteiro para a injeção.
/// \param injection_key Ponteiro para a chave da injeção.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi adicionada com sucesso, false em caso de erro.
bool utente_add_injection_at_vaccine_name_last_injection_key_list(Utente* utente, Application* app, Injection* injection, char* injection_key, ErrorState* estate);

/// \private \memberof Utente
/// \brief Adiciona nova data de injeção a lista de injeções aplicadas.
/// \param utente Ponteiro para o utente.
/// \param date Ponteiro para a data.
/// \param injection Ponteiro para a injeção.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi adicionada com sucesso, false em caso de erro.
bool utente_add_date_injection_key_list_vector(Utente* utente, Date* date, Injection* injection, ErrorState* estate);

/// \private \memberof Utente
/// \brief Adiciona nova injeção a lista de injeções aplicadas.
/// \param utente Ponteiro para o utente.
/// \param injection Ponteiro para a injeção.
/// \param injection_key Ponteiro para a chave da injeção.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi adicionada com sucesso, false em caso de erro.
bool utente_add_injection_at_injection_date_injection_list_map(Utente* utente, Injection* injection, char* injection_key, ErrorState* estate);

/// \private \memberof Utente
/// \brief Remove uma injeção de um utente.
/// \param utente Ponteiro para o utente.
/// \param vaccine_name Nome da vacina.
/// \param injection_key Ponteiro para a chave da injeção.
/// \return true se a injeção foi removida com sucesso, false em caso de erro.
void utente_remove_injection_at_vaccine_name_last_injection_key(Utente* utente, char* vaccine_name, char* injection_key);

/// \private \memberof Utente
/// \brief Remove uma injeção de um utente.
/// \param utente Ponteiro para o utente.
/// \param injection_key Ponteiro para a chave da injeção.
/// \param injection Ponteiro para a injeção.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a injeção foi removida com sucesso, false em caso de erro.
void utente_remove_injection_at_date_injection_key_list(Utente* utente, char* injection_key, Injection* injection, ErrorState* estate);

/// \private \memberof Utente
/// \brief Remove todas as injeções de um utente.
/// \param utente Ponteiro para o utente.
/// \param app Ponteiro para a aplicação.
/// \param date Ponteiro para a data.
/// \param estate Ponteiro para o estado de erro.
/// \return true se as injeções foram removidas com sucesso, false em caso de erro.
bool utente_remove_entire_day_data(Utente* utente, Application* app, Date* date, ErrorState* estate);
