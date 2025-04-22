/**
 * @file parsers.c
 * @author ist1106900 (Goncalo Aleixo)
*/

// header
#include "parsers.h"

// c stdlib
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>


/* -------------------------------------------------------------------------- */
/*                          instruction parser state                          */
/* -------------------------------------------------------------------------- */

bool instruction_parser_state_init(InstructionParserState* state, char* input_str_ref, usize input_size) {
    
    // inicializa o estado do parser
    // dado que o input se mantém vivo ao longo da execução do programa, não é necessário fazer uma cópia
    state->input_size = input_size;
    state->current_idx = 0;
    state->input_str = input_str_ref;
    return true;
}

void instruction_parser_state_free(InstructionParserState* state) {
    (void)state;
    // nada a fazer, já que nenhuma alocação é feita
}


char instruction_parser_state_peek_char(InstructionParserState* state) {
    
    // verifica se o índice atual está dentro dos limites do input
    if (state->current_idx >= state->input_size)
        return 0;

    return state->input_str[state->current_idx];
}

char instruction_parser_state_take_char(InstructionParserState* state) {
    
    // verifica se o índice atual está dentro dos limites do input
    if (state->current_idx >= state->input_size)
        return 0;

    // retorna o caractere atual e incrementa o índice
    char result = state->input_str[state->current_idx]; // caractere atual
    state->current_idx += 1;
    return result;
}

bool instruction_parser_state_try_take_digit(InstructionParserState* state, u8* dst) {
    
    // verifica se o próximo caractere é um dígito
    char ch = instruction_parser_state_peek_char(state); // próximo caractere
    if (ch == '\0' || !isdigit(ch))
        return false;

    state->current_idx += 1;

    // determina o valor numérico do primeiro dígito subtraindo o char do digito pelo char do '0'.
    // funciona já que os chars dos dígitos estão em ordem crescente na tabela ASCII.
    *dst = ch - '0';
    return true;
}

usize instruction_parser_state_calc_utf8_char_size(InstructionParserState* state) {

    // descobre quantos bytes tem o caractere UTF-8
    usize bytes = 0; // número de bytes do caractere
    if ((state->input_str[state->current_idx] & 0x80) == 0)
        bytes = 1;
    else if ((state->input_str[state->current_idx] & 0xE0) == 0xC0)
        bytes = 2;
    else if ((state->input_str[state->current_idx] & 0xF0) == 0xE0)
        bytes = 3;
    else if ((state->input_str[state->current_idx] & 0xF8) == 0xF0)
        bytes = 4;

    // verifica se há espaço suficiente no input para esse caractere ser completo
    assert(state->current_idx + bytes <= state->input_size, "Erro ao ler caractere UTF-8, não há espaço suficiente no input");

    return bytes;
}

bool instruction_parser_state_take_name(InstructionParserState* state, char** out_name, usize* out_name_size, ErrorState* estate) {
    
    // lê caracteres utf-8 até encontrar um espaço em branco ou o fim do input
    usize name_size = 0; // tamanho do nome em bytes
    char ch = instruction_parser_state_peek_char(state); // caractere atual
    while (!isblank(ch) && ch != '\0') {

        usize next_char_size = instruction_parser_state_calc_utf8_char_size(state);
        state->current_idx += next_char_size;
        name_size += next_char_size;
        ch = instruction_parser_state_peek_char(state);
    }

    // o nome tem de ter pelo menos 1 caractere
    if (name_size == 0) {
        set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    // aloca memória para o nome e copia os caracteres
    *out_name = malloc(name_size + 1);
    if (*out_name == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }
    memcpy(*out_name, state->input_str + state->current_idx - name_size, name_size);
    (*out_name)[name_size] = '\0';
    *out_name_size = name_size;
    
    return true;
}

void instruction_parser_consume_blanks(InstructionParserState* state) {
    
    // consome todos os espaços em branco até encontrar um caractere diferente de espaço em branco
    // ou o fim do input

    char ch = instruction_parser_state_peek_char(state); // próximo caractere
    while (isblank(ch) && ch != '\0') {
        state->current_idx += 1;
        ch = instruction_parser_state_peek_char(state);
    }
}

/* -------------------------------------------------------------------------- */
/*                          instruction data parsers                          */
/* -------------------------------------------------------------------------- */

bool parse_utente_name_core_internal(InstructionParserState* state, bool is_complex_name, usize* out_name_size, ErrorState* estate) {
    
    // caso o nome seja complexo, precisamos consumir caracteres em branco antes de ler nomes
    while (is_complex_name && isblank(instruction_parser_state_peek_char(state))) {
        state->current_idx += 1;
        *out_name_size += 1;
    }

    // lê o nome até encontrar um espaço em branco ou o fim do input(ou aspas duplas, se for um nome complexo)
    char* name;          // ponteiro para o nome
    usize name_size = 0; // tamanho do nome
    if (!instruction_parser_state_take_name(state, &name, &name_size, estate))
        return false;

    // incrementa o tamanho do nome com o tamanho do nome lido
    *out_name_size += name_size;
    if (name[name_size - 1] == '"') {
        state->current_idx -= 1;
        *out_name_size -= 1;
    }

    free(name);
    return true;
}

bool parse_utente_name_core(InstructionParserState* state, usize* out_name_size, char** out_name_start, ErrorState* estate) {

    // checa pela presença de aspas duplas, indicando um nome complexo que pode conter espaços em branco
    bool is_complex_name = false; // indica se o nome é complexo
    if (instruction_parser_state_peek_char(state) == '"') {
        is_complex_name = true;
        state->current_idx += 1;
    }

    *out_name_start = state->input_str + state->current_idx;
    char ch = instruction_parser_state_peek_char(state); // próximo caractere
    while ((is_complex_name) ? (ch != '"') : (!isblank(ch) && ch != '\0')) {
        
        if (!parse_utente_name_core_internal(state, is_complex_name, out_name_size, estate)) {
            if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
            return false;
        }
        ch = instruction_parser_state_peek_char(state);
    }

    // se o nome é complexo, o último caractere tem de ser uma aspa dupla, dá mesmo forma que havia no início
    if (is_complex_name) {
        if (instruction_parser_state_take_char(state) != '"') {
            set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
            return false;
        }
    }

    return true;
}

bool parse_utente_name(InstructionParserState* state, char** dst, ErrorState* estate) {
    
    usize name_size = 0; // tamanho do nome
    char* name_start = state->input_str + state->current_idx; // ponteiro para o início do nome
    if (!parse_utente_name_core(state, &name_size, &name_start, estate)) {
        set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    // o nome tem de ter pelo menos 1 caractere
    if (name_size == 0) {
        set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    // aloca memória para o nome e copia os caracteres
    *dst = malloc(name_size + 1);
    if (*dst == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }
    memcpy(*dst, name_start, name_size);
    (*dst)[name_size] = '\0';
    
    return true;
}


bool parse_vaccine_name(InstructionParserState* state, char** dst, ErrorState* estate) {
    
    // lê o nome da vacina até encontrar um espaço em branco ou o fim do input
    usize name_size; // tamanho do nome
    if (!instruction_parser_state_take_name(state, dst, &name_size, estate))
        return false;
     
    // verifica se o nome da vacina respeita os limites de tamanho
    if (name_size > VACCINE_NAME_MAX_LENGTH) {
        free(*dst);
        if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    return true;
}

void handle_batch_name_composition_error(InstructionParserState* state, usize batch_size, ErrorState* estate) {

    if (estate->cmd_ch == 'r') {
        // se o comando é retirar lote, então o nome recebe um tratamento diferente
        char stz = state->input_str[state->current_idx];
        state->input_str[state->current_idx] = '\0';
        set_error(ERROR_NO_SUCH_BATCH, estate, (state->input_str + (state->current_idx - batch_size)), NULL, NULL);
        state->input_str[state->current_idx] = stz;
    } else {
        set_error(ERROR_INVALID_BATCH, estate, NULL, NULL, NULL);
    }
}

bool parse_batch_name(InstructionParserState* state, char** dst, ErrorState* estate) {
    
    // determina o tamanho do nome do lote
    usize batch_size = 0; // tamanho do nome do lote
    char ch = instruction_parser_state_peek_char(state);
    while (!isblank(ch) && ch != '\0') {
        batch_size += 1;
        state->current_idx += 1;
        ch = instruction_parser_state_peek_char(state);
    }

    // o nome do lote tem de ter pelo menos 1 caractere
    if (batch_size == 0 || batch_size > BATCH_NAME_MAX_LENGTH) {
        set_error(ERROR_INVALID_BATCH, estate, NULL, NULL, NULL);
        return false;
    }

    // valida se o nome do lote é composto apenas por dígitos hexadecimais
    for (usize i = 0; i < batch_size; ++i) {
        char ch = state->input_str[state->current_idx - batch_size + i];
        if (!(isdigit(ch) || (ch >= 'A' && ch <= 'F'))) {
            
            handle_batch_name_composition_error(state, batch_size, estate);
            return false;
        }
    }

    // aloca memória para o nome do lote e copia os caracteres
    *dst = malloc(batch_size + 1);
    if (*dst == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }
    memcpy(*dst, state->input_str + state->current_idx - batch_size, batch_size);
    (*dst)[batch_size] = '\0';
    
    return true;
}   


bool parse_dose_number(InstructionParserState* state, u32* dst, ErrorState* estate) {
    
    usize number_start_idx = state->current_idx;
    
    // lê dígitos até encontrar um espaço em branco ou o fim do input
    u32 result = 0; // número
    u8 digit; // dígito lido
    while (instruction_parser_state_try_take_digit(state, &digit))
        result = result * 10 + digit;

    // se não foi lido nenhum dígito, então não é um número
    if (state->current_idx == number_start_idx) {
        set_error(ERROR_INVALID_QUANTITY, estate, NULL, NULL, NULL);
        return false;
    }

    // escreve o resultado no destino e retorna sucesso
    *dst = result;
    return true;
}


/* ------------------------------ date parsing ------------------------------ */

bool parse_two_digit_number(InstructionParserState* state, u8* dst) {

    u8 result = 0; // resultado, número de dois dígitos

    // lê o primeiro dígito, que é obrigatório
    u8 digit; // dígito lido
    if (!instruction_parser_state_try_take_digit(state, &digit)) {
        internal_error("Erro ao parsear primeiro dígito de número de dois dígitos");
        return false;
    }
    result = digit;

    // tenta ler o segundo dígito, que é opcional
    // caso ele exista, fazemos o shift decimal e somamos para inserir o novo digito
    if (instruction_parser_state_try_take_digit(state, &digit))
        result = result * 10 + digit;

    // escreve o resultado no destino e retorna sucesso
    *dst = result;
    return true;
}

bool parse_year_number(InstructionParserState* state, u32* dst, ErrorState* estate) {

    usize year_start = state->current_idx; // índice do início do ano
    usize last_digit = state->current_idx; // índice do último dígito lido
    while (isdigit(instruction_parser_state_peek_char(state)))
        last_digit = state->current_idx++;

    // o ano tem de ter pelo menos 1 dígito
    if ((last_digit - year_start) == 0) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        return false;
    }

    char* strtoul_result;
    u32 year = strtoul(state->input_str + year_start, &strtoul_result, 10);
    if ((strtoul_result != state->input_str + last_digit + 1)) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL); // provavelmente nunca deveria acontecer
        return false;
    }

    *dst = year;
    return true;
}

bool parse_owned_date(InstructionParserState* state, Date** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    Date* date = NULL; // ponteiro onde a data será armazenada
    if (isdigit(instruction_parser_state_peek_char(state))) {
        
        Date d; // data temporária
        if (!parse_date(state, &d, estate)) {
            if (!estate->error_emitted) set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
            return false;
        }

        // aloca memória para a data e copia os dados
        date = malloc(sizeof(Date));
        if (date == NULL) {
            set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
            return false;
        }
        
        *date = d;
    }

    *dst = date;
    return true;
}

#define CHECK_FOR_DATE_SEPARATOR(state, estate) \
    if (instruction_parser_state_take_char((state)) != '-') { \
        set_error(ERROR_INVALID_DATE, (estate), NULL, NULL, NULL); \
        return false; \
    }

#define PARSE_DATE_TWO_DIGIT_NUMBER(state, dst) \
    if (isdigit(instruction_parser_state_peek_char((state))) && !parse_two_digit_number((state), (dst))) { \
        set_error(ERROR_INVALID_DATE, (estate), NULL, NULL, NULL); \
        return false; \
    }

bool parse_date(InstructionParserState* state, Date* dst, ErrorState* estate) {
    
    Date result = {0, 0, 0}; // resultado a ser retornado

    // leitura do dia
    PARSE_DATE_TWO_DIGIT_NUMBER(state, &result.day);
    CHECK_FOR_DATE_SEPARATOR(state, estate);
    PARSE_DATE_TWO_DIGIT_NUMBER(state, &result.month);
    CHECK_FOR_DATE_SEPARATOR(state, estate);

    if (!parse_year_number(state, &result.year, estate)) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        return false;
    }

    // checa se a data é válida, verificando requerimentos semâncticos de cada campo
    if (!date_is_valid(&result)) {
        set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        return false;
    }

    *dst = result;
    return true;
}

#undef CHECK_FOR_DATE_SEPARATOR
#undef PARSE_DATE_TWO_DIGIT_NUMBER

/* -------------------------------------------------------------------------- */
/*                          instruction body parsers                          */
/* -------------------------------------------------------------------------- */

bool parse_instruction_terminar(InstructionParserState* state, InstructionBody_Terminar** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);
    if (instruction_parser_state_peek_char(state) != '\0') {
        internal_error("Erro ao parsear instrução terminar, caracteres inesperados");
        return false;
    }

    if ((*dst = instruction_body_terminar_new(estate)) == NULL)
        return false;

    return true;
}

bool parse_instruction_criar_lote(InstructionParserState* state, InstructionBody_CriarLote** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    char* batch_name; // ponteiro onde o nome do lote será armazenado
    if (!parse_batch_name(state, &batch_name, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_BATCH, estate, NULL, NULL, NULL);
        return false;
    }

    instruction_parser_consume_blanks(state);

    Date expiration_date; // data de validade do lote
    if (!parse_date(state, &expiration_date, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_DATE, estate, NULL, NULL, NULL);
        free(batch_name); return false;
    }

    instruction_parser_consume_blanks(state);

    u32 quantity; // quantidade de doses do lote
    if (!parse_dose_number(state, &quantity, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_QUANTITY, estate, NULL, NULL, NULL);
        free(batch_name); return false;
    }

    instruction_parser_consume_blanks(state);

    char* vaccine_name; // ponteiro onde o nome da vacina será armazenado
    if (!parse_vaccine_name(state, &vaccine_name, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        free(batch_name); return false;
    }

    instruction_parser_consume_blanks(state);
    assert(instruction_parser_state_peek_char(state) == '\0', "Erro ao parsear instrução criar lote, caracteres inesperados");

    if ((*dst = instruction_body_criar_lote_new(batch_name, expiration_date, quantity, vaccine_name, estate)) == NULL) return false;
    return true;
}


bool parse_instruction_listar_vacinas(InstructionParserState* state, InstructionBody_ListarVacinas** dst, ErrorState* estate) {

    Vector vaccines; // vetor onde os nomes das vacinas serão armazenados
    vector_init(&vaccines, sizeof(char*), vector_string_free);

    while (true) {

        instruction_parser_consume_blanks(state);

        if (instruction_parser_state_peek_char(state) == '\0')
            break;

        char* vaccine_name; // ponteiro onde o nome da vacina será armazenado
        if (!parse_vaccine_name(state, &vaccine_name, estate)) {
            vector_free(&vaccines);
            if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
            return false;
        }

        if (!vector_insert(&vaccines, vector_size(&vaccines), &vaccine_name, estate)) {
            vector_free(&vaccines);
            return false;
        }
    }

    if ((*dst = instruction_body_listar_vacinas_new(vaccines, estate)) == NULL)
        return false;

    return true;
}


bool parse_instruction_aplicar_dose(InstructionParserState* state, InstructionBody_AplicarDose** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    char* utente_name; // ponteiro onde o nome do utente será armazenado
    if (!parse_utente_name(state, &utente_name, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    instruction_parser_consume_blanks(state);

    char* vaccine_name; // ponteiro onde o nome da vacina será armazenado
    if (!parse_vaccine_name(state, &vaccine_name, estate)) {
        free(utente_name);
        if (!estate->error_emitted) set_error(ERROR_NO_STOCK, estate, NULL, NULL, NULL);
        return false;
    }

    instruction_parser_consume_blanks(state);
    if (instruction_parser_state_peek_char(state) != '\0') {
        free(utente_name);
        free(vaccine_name);
        internal_error("Erro ao parsear instrução aplicar dose, caracteres inesperados");
        return false;
    }

    if ((*dst = instruction_body_aplicar_dose_new(utente_name, vaccine_name, estate)) == NULL)
        return false;

    return true;
}


bool parse_instruction_retirar_lote(InstructionParserState* state, InstructionBody_RetirarLote** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    char* batch_name; // ponteiro onde o nome do lote será armazenado
    if (!parse_batch_name(state, &batch_name, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_BATCH, estate, NULL, NULL, NULL);
        return false;
    }

    instruction_parser_consume_blanks(state);
    if (instruction_parser_state_peek_char(state) != '\0') {
        free(batch_name);
        internal_error("Erro ao parsear instrução retirar lote, caracteres inesperados");
        return false;
    }

    if ((*dst = instruction_body_retirar_lote_new(batch_name, estate)) == NULL)
        return false;

    return true;
}



bool parse_instruction_deletar_registro(InstructionParserState* state, InstructionBody_DeletarRegistro** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    char* utente_name; // ponteiro onde o nome do utente será armazenado
    if (!parse_utente_name(state, &utente_name, estate)) {
        if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
        return false;
    }

    Date* application_date = NULL; // ponteiro onde a data da aplicação será armazenada
    parse_owned_date(state, &application_date, estate);

    instruction_parser_consume_blanks(state);

    char* batch_name = NULL; // ponteiro onde o nome do lote será armazenado
    if (instruction_parser_state_peek_char(state) != '\0' && !isblank(instruction_parser_state_peek_char(state))) {
        if (!parse_batch_name(state, &batch_name, estate)) {
            free(utente_name); free(application_date);
            if (!estate->error_emitted) set_error(ERROR_INVALID_BATCH, estate, NULL, NULL, NULL);
            return false;
        }
    }

    instruction_parser_consume_blanks(state);
    assert(instruction_parser_state_peek_char(state) == '\0', "Erro ao parsear instrução deletar registro, caracteres inesperados");

    if ((*dst = instruction_body_deletar_registro_new(utente_name, application_date, batch_name, estate)) == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    return true;
}


bool parse_instruction_listar_aplicações(InstructionParserState* state, InstructionBody_ListarAplicações** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    char* utente_name = NULL; // ponteiro onde o nome do utente será armazenado
    char next_char = instruction_parser_state_peek_char(state); // próximo caractere
    if (!isblank(next_char) && next_char != '\0') {
        if (!parse_utente_name(state, &utente_name, estate)) {
            if (!estate->error_emitted) set_error(ERROR_INVALID_NAME, estate, NULL, NULL, NULL);
            return false;
        }
    }

    instruction_parser_consume_blanks(state);
    if (instruction_parser_state_peek_char(state) != '\0') {
        free(utente_name);
        internal_error("Erro ao parsear instrução listar aplicações, caracteres inesperados");
        return false;
    }

    if ((*dst = instruction_body_listar_aplicações_new(utente_name, estate)) == NULL)
        return false;

    return true;
}


bool parse_instruction_avançar_tempo(InstructionParserState* state, InstructionBody_AvançarTempo** dst, ErrorState* estate) {

    instruction_parser_consume_blanks(state);

    Date* new_date = NULL; // ponteiro onde a nova data será armazenada
    if (!parse_owned_date(state, &new_date, estate))
        return false;

    instruction_parser_consume_blanks(state);
    if (instruction_parser_state_peek_char(state) != '\0') {
        free(new_date);
        internal_error("Erro ao parsear instrução avançar tempo, caracteres inesperados");
        return false;
    }

    if ((*dst = instruction_body_avançar_tempo_new(new_date, estate)) == NULL)
        return false;

    return true;
}


bool command_q(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_TERMINAR;
    if (!parse_instruction_terminar(state, (InstructionBody_Terminar**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }
    

    return true;
}

bool command_c(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_CRIAR_LOTE;
    if (!parse_instruction_criar_lote(state, (InstructionBody_CriarLote**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_l(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_LISTAR_VACINAS;
    if (!parse_instruction_listar_vacinas(state, (InstructionBody_ListarVacinas**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_a(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_APLICAR_DOSE;
    if (!parse_instruction_aplicar_dose(state, (InstructionBody_AplicarDose**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_r(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_RETIRAR_LOTE;
    if (!parse_instruction_retirar_lote(state, (InstructionBody_RetirarLote**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_d(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_DELETAR_REGISTRO;
    if (!parse_instruction_deletar_registro(state, (InstructionBody_DeletarRegistro**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_u(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_LISTAR_APLICAÇÕES;
    if (!parse_instruction_listar_aplicações(state, (InstructionBody_ListarAplicações**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}

bool command_t(InstructionParserState* state, Instruction* dst, ErrorState* estate) {

    dst->type = INSTRUCTION_TYPE_AVANÇAR_TEMPO;
    if (!parse_instruction_avançar_tempo(state, (InstructionBody_AvançarTempo**)&dst->body, estate)) {
        instruction_parser_state_free(state);
        return false;
    }

    return true;
}


void dispatch_instruction_parsing(InstructionParserState* state, char cmd_ch, Instruction* dst, bool* result, ErrorState* estate) {

    switch (cmd_ch) {
        case 'q':
            *result = command_q(state, dst, estate);
            break;

        case 'c':
            *result = command_c(state, dst, estate);
            break;

        case 'l':
            *result = command_l(state, dst, estate);
            break;

        case 'a':
            *result = command_a(state, dst, estate);
            break;

        case 'r':
            *result = command_r(state, dst, estate);
            break;

        case 'd':
            *result = command_d(state, dst, estate);
            break;

        case 'u':
            *result = command_u(state, dst, estate);
            break;

        case 't':
            *result = command_t(state, dst, estate);
            break;
        
        default:
            *result = false;
    }
}


bool parse_instruction(char *input, usize input_size, Instruction* dst, ErrorState* estate, bool* jump) {

    InstructionParserState state; // estado do parser
    instruction_parser_state_init(&state, input, input_size);

    char cmd_ch = instruction_parser_state_take_char(&state); // caractere do comando
    if (cmd_ch == '\0') {
        internal_error("Erro ao parsear instrução, comando não encontrado");
        return false;
    }

    // desconsidera o comando caso ele não tenha um caractere inicial válido e separado de seu corpo
    if (state.input_size > 1 && instruction_parser_state_peek_char(&state) != ' ') {
        instruction_parser_state_free(&state);
        *jump = true;
        return true;
    }
    
    estate->cmd_ch = cmd_ch;

    Instruction instruction; // instrução a ser retornada
    bool result; // resultado da operação
    dispatch_instruction_parsing(&state, cmd_ch, &instruction, &result, estate);

    if (!result) {
        instruction_parser_state_free(&state);
        return result;
    }

    instruction_parser_state_free(&state);
    *dst = instruction;
    *jump = false;
    return true;
}



/* -------------------------------------------------------------------------- */
/*                                instructions                                */
/* -------------------------------------------------------------------------- */

/* -------------------------------- terminar -------------------------------- */

InstructionBody_Terminar* instruction_body_terminar_new(ErrorState* estate) {
    
    InstructionBody_Terminar* body = malloc(sizeof(InstructionBody_Terminar)); // memória para o corpo da instrução
    if (body == NULL) {
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    return body;
}

void instruction_body_terminar_free(InstructionBody_Terminar* body) {
    free(body);
}


/* ------------------------------- criar lote ------------------------------- */

InstructionBody_CriarLote* instruction_body_criar_lote_new(char* batch_name, Date expiration_date, u32 dose_count, char* vaccine_name, ErrorState* estate) {
    
    InstructionBody_CriarLote* body = malloc(sizeof(InstructionBody_CriarLote)); // memória para o corpo da instrução
    if (body == NULL) {
        free(batch_name);
        free(vaccine_name);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->batch_name = batch_name;
    body->expiration_date = expiration_date;
    body->dose_count = dose_count;
    body->vaccine_name = vaccine_name;
    return body;
}

void instruction_body_criar_lote_free(InstructionBody_CriarLote* body) {
    free(body->batch_name);
    free(body->vaccine_name);
    free(body);
}


/* ----------------------------- listar vacinas ----------------------------- */

InstructionBody_ListarVacinas* instruction_body_listar_vacinas_new(Vector vaccine_names, ErrorState* estate) {

    InstructionBody_ListarVacinas* body = malloc(sizeof(InstructionBody_ListarVacinas)); // memória para o corpo da instrução
    if (body == NULL) {
        vector_free(&vaccine_names);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->vaccine_names = vaccine_names;
    return body;
}

void instruction_body_listar_vacinas_free(InstructionBody_ListarVacinas* body) {
    vector_free(&body->vaccine_names);
    free(body);
}


/* ------------------------------ aplicar dose ------------------------------ */

InstructionBody_AplicarDose* instruction_body_aplicar_dose_new(char* utente_name, char* vaccine_name, ErrorState* estate) {

    InstructionBody_AplicarDose* body = malloc(sizeof(InstructionBody_AplicarDose)); // memória para o corpo da instrução
    if (body == NULL) {
        free(utente_name);
        free(vaccine_name);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->utente_name = utente_name;
    body->vaccine_name = vaccine_name;
    return body;
}

void instruction_body_aplicar_dose_free(InstructionBody_AplicarDose* body) {
    free(body->utente_name);
    free(body->vaccine_name);
    free(body);
}


/* ------------------------------ retirar lote ------------------------------ */

InstructionBody_RetirarLote* instruction_body_retirar_lote_new(char* batch_name, ErrorState* estate) {

    InstructionBody_RetirarLote* body = malloc(sizeof(InstructionBody_RetirarLote)); // memória para o corpo da instrução
    if (body == NULL) {
        free(batch_name);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->batch_name = batch_name;
    return body;
}

void instruction_body_retirar_lote_free(InstructionBody_RetirarLote* body) {
    free(body->batch_name);
    free(body);
}


/* ---------------------------- deletar registro ---------------------------- */

InstructionBody_DeletarRegistro* instruction_body_deletar_registro_new(char* utente_name, Date* application_date, char* batch_name, ErrorState* estate) {

    InstructionBody_DeletarRegistro* body = malloc(sizeof(InstructionBody_DeletarRegistro)); // memória para o corpo da instrução
    if (body == NULL) {
        free(utente_name);
        free(application_date);
        free(batch_name);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->utente_name = utente_name;
    body->application_date = application_date;
    body->batch_name = batch_name;
    return body;
}

void instruction_body_deletar_registro_free(InstructionBody_DeletarRegistro* body) {
    free(body->utente_name);
    free(body->application_date);
    free(body->batch_name);
    free(body);
}


/* ---------------------------- listar aplicações --------------------------- */

InstructionBody_ListarAplicações* instruction_body_listar_aplicações_new(char* utente_name, ErrorState* estate) {

    InstructionBody_ListarAplicações* body = malloc(sizeof(InstructionBody_ListarAplicações)); // memória para o corpo da instrução
    if (body == NULL) {
        free(utente_name);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->utente_name = utente_name;
    return body;
}

void instruction_body_listar_aplicações_free(InstructionBody_ListarAplicações* body) {
    free(body->utente_name);
    free(body);
}


/* ------------------------------ avançar tempo ----------------------------- */

InstructionBody_AvançarTempo* instruction_body_avançar_tempo_new(Date* new_date, ErrorState* estate) {

    InstructionBody_AvançarTempo* body = malloc(sizeof(InstructionBody_AvançarTempo)); // memória para o corpo da instrução
    if (body == NULL) {
        free(new_date);
        set_error(ERROR_NO_MEMORY, estate, NULL, NULL, NULL);
        return false;
    }

    body->new_date = new_date;
    return body;
}

void instruction_body_avançar_tempo_free(InstructionBody_AvançarTempo* body) {
    free(body->new_date);
    free(body);
}
