/**
 * @file parsers.h
 * @author ist1106900 (Goncalo Aleixo)
*/

#pragma once

/// \brief Cabeçalho para a definição de instruções e do parsers de instruções.

// local
#include "core.h"
#include "data_types.h"



/* -------------------------------------------------------------------------- */
/*                                instructions                                */
/* -------------------------------------------------------------------------- */

/// \enum InstructionType
/// \brief Enum para representar cada tipo de instrução.
typedef enum {
    INSTRUCTION_TYPE_TERMINAR,
    INSTRUCTION_TYPE_CRIAR_LOTE,
    INSTRUCTION_TYPE_LISTAR_VACINAS,
    INSTRUCTION_TYPE_APLICAR_DOSE,
    INSTRUCTION_TYPE_RETIRAR_LOTE,
    INSTRUCTION_TYPE_DELETAR_REGISTRO,
    INSTRUCTION_TYPE_LISTAR_APLICAÇÕES,
    INSTRUCTION_TYPE_AVANÇAR_TEMPO,
} InstructionType;


/**
 * \class Instruction
 * \brief Classe para representar uma instrução. O corpo deve ser interpretado
    * de acordo com o tipo da instrução.
 */
typedef struct {
    InstructionType type; //!< tipo da instrução
    void* body; //!< ponteiro para o corpo da instrução, de ownership próprio
} Instruction;


/* -------------------------------- terminar -------------------------------- */

/// \class InstructionBody_Terminar
/// \brief Estrutura para representar o corpo da instrução 'terminar'.
typedef struct {
    u8 placeholder; //!< placeholder, não é usado, mas é necessário para manter a estrutura válida
} InstructionBody_Terminar;

/// \public \memberof InstructionBody_Terminar
/// \brief Cria um novo corpo de instrução 'terminar'.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'terminar', ou NULL em caso de erro.
InstructionBody_Terminar* instruction_body_terminar_new(ErrorState* estate);

/// \public \memberof InstructionBody_Terminar
/// \brief Libera a memória do corpo da instrução 'terminar'.
/// \param body Ponteiro para o corpo da instrução 'terminar'.
void instruction_body_terminar_free(InstructionBody_Terminar* body);


/* ------------------------------- criar lote ------------------------------- */

/// \class InstructionBody_CriarLote
/// \brief Estrutura para representar o corpo da instrução 'criar lote'.
typedef struct {
    char* batch_name; //!< nome do lote, de ownership próprio
    Date expiration_date; //!< data de validade do lote
    u32 dose_count; //!< número de doses do lote
    char* vaccine_name; //!< nome da vacina, de ownership próprio
} InstructionBody_CriarLote;

/// \public \memberof InstructionBody_CriarLote
/// \brief Cria um novo corpo de instrução 'criar lote'.
/// \param batch_name Nome do lote. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param expiration_date Data de validade do lote.
/// \param dose_count Número de doses do lote.
/// \param vaccine_name Nome da vacina. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'criar lote', ou NULL em caso de erro.
InstructionBody_CriarLote* instruction_body_criar_lote_new(char* batch_name, Date expiration_date, u32 dose_count, char* vaccine_name, ErrorState* estate);

/// \public \memberof InstructionBody_CriarLote
/// \brief Libera a memória do corpo da instrução 'criar lote'.
/// \param body Ponteiro para o corpo da instrução 'criar lote'.
void instruction_body_criar_lote_free(InstructionBody_CriarLote* body);


/* ----------------------------- listar vacinas ----------------------------- */

/// \class InstructionBody_ListarVacinas
/// \brief Estrutura para representar o corpo da instrução 'listar vacinas'.
typedef struct {
    Vector vaccine_names; //!< vetor de nomes de vacinas, de ownership próprio
} InstructionBody_ListarVacinas;

/// \public \memberof InstructionBody_ListarVacinas
/// \brief Cria um novo corpo de instrução 'listar vacinas'.
/// \param vaccine_names Vetor de nomes de vacinas. O vetor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'listar vacinas', ou NULL em caso de erro.
InstructionBody_ListarVacinas* instruction_body_listar_vacinas_new(Vector vaccine_names, ErrorState* estate);

/// \public \memberof InstructionBody_ListarVacinas
/// \brief Libera a memória do corpo da instrução 'listar vacinas'.
/// \param body Ponteiro para o corpo da instrução 'listar vacinas'.
void instruction_body_listar_vacinas_free(InstructionBody_ListarVacinas* body);


/* ------------------------------ aplicar dose ------------------------------ */

/// \class InstructionBody_AplicarDose
/// \brief Estrutura para representar o corpo da instrução 'aplicar dose'.
typedef struct {
    char* utente_name; //!< nome do utente, de ownership próprio
    char* vaccine_name; //!< nome da vacina, de ownership próprio
} InstructionBody_AplicarDose;

/// \public \memberof InstructionBody_AplicarDose
/// \brief Cria um novo corpo de instrução 'aplicar dose'.
/// \param utente_name Nome do utente. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param vaccine_name Nome da vacina. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'aplicar dose', ou NULL em caso de erro.
InstructionBody_AplicarDose* instruction_body_aplicar_dose_new(char* utente_name, char* vaccine_name, ErrorState* estate);

/// \public \memberof InstructionBody_AplicarDose
/// \brief Libera a memória do corpo da instrução 'aplicar dose'.
/// \param body Ponteiro para o corpo da instrução 'aplicar dose'.
void instruction_body_aplicar_dose_free(InstructionBody_AplicarDose* body);


/* ------------------------------ retirar lote ------------------------------ */

/// \class InstructionBody_RetirarLote
/// \brief Estrutura para representar o corpo da instrução 'retirar lote'.
typedef struct {
    char* batch_name; //!< nome do lote, de ownership próprio
} InstructionBody_RetirarLote;

/// \public \memberof InstructionBody_RetirarLote
/// \brief Cria um novo corpo de instrução 'retirar lote'.
/// \param batch_name Nome do lote. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'retirar lote', ou NULL em caso de erro.
InstructionBody_RetirarLote* instruction_body_retirar_lote_new(char* batch_name, ErrorState* estate);

/// \public \memberof InstructionBody_RetirarLote
/// \brief Libera a memória do corpo da instrução 'retirar lote'.
/// \param body Ponteiro para o corpo da instrução 'retirar lote'.
void instruction_body_retirar_lote_free(InstructionBody_RetirarLote* body);


/* ---------------------------- deletar registro ---------------------------- */

/// \class InstructionBody_DeletarRegistro
/// \brief Estrutura para representar o corpo da instrução 'deletar registro'.
typedef struct {
    char* utente_name; //!< nome do utente, de ownership próprio
    Date* application_date; //!< data da aplicação, de ownership próprio, ou NULL
    char* batch_name; //!< nome do lote, de ownership próprio, ou NULL
} InstructionBody_DeletarRegistro;

/// \public \memberof InstructionBody_DeletarRegistro
/// \brief Cria um novo corpo de instrução 'deletar registro'.
/// \param utente_name Nome do utente. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param application_date Data da aplicação, opcional. O valor é tomado, logo, o ownership não é mais do chamador.
///     \note O valor pode ser NULL, caso não seja necessário.
/// \param batch_name Nome do lote, opcional. O valor é tomado, logo, o ownership não é mais do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'deletar registro', ou NULL em caso de erro.
InstructionBody_DeletarRegistro* instruction_body_deletar_registro_new(char* utente_name, Date* application_date, char* batch_name, ErrorState* estate);

/// \public \memberof InstructionBody_DeletarRegistro
/// \brief Libera a memória do corpo da instrução 'deletar registro'.
/// \param body Ponteiro para o corpo da instrução 'deletar registro'.
void instruction_body_deletar_registro_free(InstructionBody_DeletarRegistro* body);


/* ---------------------------- listar aplicações --------------------------- */

/// \class InstructionBody_ListarAplicações
/// \brief Estrutura para representar o corpo da instrução 'listar aplicações'.
typedef struct {
    char* utente_name; //!< nome do utente, de ownership próprio, ou NULL
} InstructionBody_ListarAplicações;

/// \public \memberof InstructionBody_ListarAplicações
/// \brief Cria um novo corpo de instrução 'listar aplicações'.
/// \param utente_name Nome do utente, opcional. O valor é tomado, logo, o ownership não é mais do chamador.
///     \note O valor pode ser NULL, caso não seja necessário.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'listar aplicações', ou NULL em caso de erro.
InstructionBody_ListarAplicações* instruction_body_listar_aplicações_new(char* utente_name, ErrorState* estate);

/// \public \memberof InstructionBody_ListarAplicações
/// \brief Libera a memória do corpo da instrução 'listar aplicações'.
/// \param body Ponteiro para o corpo da instrução 'listar aplicações'.
void instruction_body_listar_aplicações_free(InstructionBody_ListarAplicações* body);


/* ------------------------------ avançar tempo ----------------------------- */

/// \class InstructionBody_AvançarTempo
/// \brief Estrutura para representar o corpo da instrução 'avançar tempo'.
typedef struct {
    Date* new_date; //!< nova data, de ownership próprio, ou NULL
} InstructionBody_AvançarTempo;

/// \public \memberof InstructionBody_AvançarTempo
/// \brief Cria um novo corpo de instrução 'avançar tempo'.
/// \param new_date Nova data, opcional. O valor é tomado, logo, o ownership não é mais do chamador.
///     \note O valor pode ser NULL, caso não seja necessário.
/// \param estate Ponteiro para o estado de erro.
/// \return Ponteiro para o novo corpo de instrução 'avançar tempo', ou NULL em caso de erro.
InstructionBody_AvançarTempo* instruction_body_avançar_tempo_new(Date* new_date, ErrorState* estate);

/// \public \memberof InstructionBody_AvançarTempo
/// \brief Libera a memória do corpo da instrução 'avançar tempo'.
/// \param body Ponteiro para o corpo da instrução 'avançar tempo'.
void instruction_body_avançar_tempo_free(InstructionBody_AvançarTempo* body);



/* -------------------------------------------------------------------------- */
/*                             instruction parser                             */
/* -------------------------------------------------------------------------- */

/// \class InstructionParserState
/// \brief Estrutura para representar o estado do parser de instruções.
typedef struct {
    char* input_str; //!< string do input, não é de ownership próprio
    usize input_size; //!< tamanho da string do input
    usize current_idx; //!< índice atual do parser na string do input
} InstructionParserState;

/* ------------------------------- ctor & dtor ------------------------------ */

/// \public \memberof InstructionParserState
/// \brief Inicializa o estado do parser de instruções.
/// \param state Ponteiro para a estrutura de estado do parser que será inicializada.
/// \param input_str String do input. O valor não é tomado, logo, o ownership é do chamador.
///     \note O valor deve ser uma string UTF-8 válida.
///     \note O valor deve permanecer válido durante toda a execução do parser.
/// \param input_size Tamanho da string do input.
/// \return true se a inicialização foi bem-sucedida, false caso contrário.
bool instruction_parser_state_init(InstructionParserState* state, char* input_str, usize input_size);

/// \public \memberof InstructionParserState
/// \brief Libera a memória do estado do parser de instruções.
/// \param state Ponteiro para a estrutura de estado do parser que será liberada.
///     \note A memória onde o estado do parser foi alocada não é de ownership próprio,
///           logo, não deve ser liberada.
void instruction_parser_state_free(InstructionParserState* state);

/* ---------------------------------- utils --------------------------------- */

/// \private \memberof InstructionParserState
/// \brief Retorna o caractere atual do parser sem avançar o índice.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \return O caractere atual do parser.
///     \note Se o índice atual estiver fora dos limites do input, retorna 0.
char instruction_parser_state_peek_char(InstructionParserState* state);

/// \public \memberof InstructionParserState
/// \brief Avança o índice do parser e retorna o caractere atual.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \return O caractere atual do parser.
///     \note Se o índice atual estiver fora dos limites do input, retorna 0.
char instruction_parser_state_take_char(InstructionParserState* state);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear um dígito, avançando o índice do parser em caso de sucesso.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o dígito parseado será armazenado.
/// \return true se o dígito foi parseado com sucesso, false caso contrário.
///     \note Se o índice atual estiver fora dos limites do input, retorna false.
bool instruction_parser_state_try_take_digit(InstructionParserState* state, u8* dst);

/// \private \memberof InstructionParserState
/// \brief Calcula o tamanho do caractere UTF-8 atual.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \return O tamanho do caractere UTF-8 atual.
///     \note Provoca um erro se o índice atual estiver fora dos limites do input.
usize instruction_parser_state_calc_utf8_char_size(InstructionParserState* state);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear um nome, avançando o índice do parser em caso de sucesso.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] out_name Ponteiro para onde o nome parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param[out] out_name_size Tamanho do nome parseado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false caso contrário.
bool instruction_parser_state_take_name(InstructionParserState* state, char** out_name, usize* out_name_size, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Consome os espaços em branco do input, até encontrar um caractere diferente de espaço em branco.
/// \param state Ponteiro para a estrutura de estado do parser.
void instruction_parser_consume_blanks(InstructionParserState* state);

/* -------------------------- general data parsers -------------------------- */

/// \private \memberof InstructionParserState
/// \brief Tenta parsear um nome de utente.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o nome parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false em caso de algum erro.
bool parse_utente_name(InstructionParserState* state, char** dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Parseia o nome de utente, retornando seu tamanho e o ponteiro para o início.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] out_name_size Tamanho do nome parseado.
/// \param[out] out_name_start Ponteiro para onde o início do nome parseado será armazenado.
///     \note O valor escrito não é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false em caso de algum erro.
bool parse_utente_name_core(InstructionParserState* state, usize* out_name_size, char** out_name_start, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Parseia o nome de utente, retornando seu tamanho.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param is_complex_name Indica se o nome é complexo.
/// \param[out] out_name_size Tamanho do nome parseado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false em caso de algum erro.
bool parse_utente_name_core_internal(InstructionParserState* state, bool is_complex_name, usize* out_name_size, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o nome da vacina.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o nome parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false em caso de algum erro.
bool parse_vaccine_name(InstructionParserState* state, char** dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o nome do lote.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o nome parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome foi parseado com sucesso, false em caso de algum erro.
bool parse_batch_name(InstructionParserState* state, char** dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o número de doses.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o número parseado será armazenado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o número foi parseado com sucesso, false em caso de algum erro.
bool parse_dose_number(InstructionParserState* state, u32* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Verifica se o nome do lote é composto apenas por dígitos hexadecimais.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param batch_size Tamanho do nome do lote.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o nome do lote é composto apenas por dígitos hexadecimais, false caso contrário.
void handle_batch_name_composition_error(InstructionParserState* state, usize batch_size, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear um número de dois dígitos.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o número parseado será armazenado.
/// \return true se o número foi parseado com sucesso, false em caso de algum erro.
bool parse_two_digit_number(InstructionParserState* state, u8* dst);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear um número de quatro dígitos.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o número parseado será armazenado.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o número foi parseado com sucesso, false em caso de algum erro.
bool parse_year_number(InstructionParserState* state, u32* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear uma data.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde a data parseada será armazenada.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a data foi parseada com sucesso, false em caso de algum erro.
bool parse_owned_date(InstructionParserState* state, Date** dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear uma data.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde a data parseada será armazenada.
/// \param estate Ponteiro para o estado de erro.
/// \return true se a data foi parseada com sucesso, false em caso de algum erro.
bool parse_date(InstructionParserState* state, Date* dst, ErrorState* estate);


/* ------------------------ instruction body parsers ------------------------ */

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'terminar'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_terminar(InstructionParserState* state, InstructionBody_Terminar** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'criar lote'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_criar_lote(InstructionParserState* state, InstructionBody_CriarLote** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'listar vacinas'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_listar_vacinas(InstructionParserState* state, InstructionBody_ListarVacinas** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'aplicar dose'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_aplicar_dose(InstructionParserState* state, InstructionBody_AplicarDose** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'retirar lote'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_retirar_lote(InstructionParserState* state, InstructionBody_RetirarLote** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'deletar registro'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_deletar_registro(InstructionParserState* state, InstructionBody_DeletarRegistro** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'listar aplicações'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_listar_aplicações(InstructionParserState* state, InstructionBody_ListarAplicações** dst, ErrorState* estate);

/// \public \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'avançar tempo'.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool parse_instruction_avançar_tempo(InstructionParserState* state, InstructionBody_AvançarTempo** dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'terminar'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_q(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'criar lote'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_c(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'listar vacinas'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_l(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'aplicar dose'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_a(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'retirar lote'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_r(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'deletar registro'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_u(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'listar aplicações'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_d(InstructionParserState* state, Instruction* dst, ErrorState* estate);

/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução 'avançar tempo'.
/// \return true se o corpo foi parseado com sucesso, false em caso de algum erro.
bool command_t(InstructionParserState* state, Instruction* dst, ErrorState* estate);


/// \private \memberof InstructionParserState
/// \brief Tenta parsear o corpo da instrução, com base no caractere de comando.
/// \param state Ponteiro para a estrutura de estado do parser.
/// \param cmd_ch Caractere de comando.
/// \param[out] dst Ponteiro para onde o corpo parseado será armazenado.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \param[out] result Ponteiro para onde o resultado da operação será armazenado.
///     \note true se o corpo foi parseado com sucesso, false em caso de erro.
void dispatch_instruction_parsing(InstructionParserState* state, char cmd_ch, Instruction* dst, bool* result, ErrorState* estate);

/// \headerfile parsers.h
/// \brief Tenta parsear uma instrução.
/// \param input String de entrada.
/// \param input_size Tamanho da string de entrada.
/// \param[out] dst Ponteiro para onde a instrução parseada será armazenada.
///     \note O valor escrito é de ownership do chamador.
/// \param estate Ponteiro para o estado de erro.
/// \param[out] jump Ponteiro para onde o valor indicando se a instrução deve ser pulada será armazenado.
/// \return true se a instrução foi parseada com sucesso, false em caso de erro.
bool parse_instruction(char* input, usize input_size, Instruction* dst, ErrorState* estate, bool* jump);
