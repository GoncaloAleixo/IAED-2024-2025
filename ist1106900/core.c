/**
 * @file core.h
 * @author ist1106900 (Goncalo Aleixo)
*/

// header
#include "core.h"

// c stdlib
#include <stdio.h>
#include <stdlib.h>



/* -------------------------------------------------------------------------- */
/*                               error handling                               */
/* -------------------------------------------------------------------------- */

#define ERROR_CASE_HANDLING(enum_item, en_str, pt_str) \
    case enum_item: \
        if (estate->pt_mode) \
            puts(pt_str); \
        else \
            puts(en_str); \
        return;

void print_error_string(Error error, ErrorState* estate, char* batch_name, char* vaccine_name, char* utente_name) {

    switch (error) {

        ERROR_CASE_HANDLING(ERROR_TOO_MANY_VACCINES, "too many vaccines", "demasiadas vacinas")
        ERROR_CASE_HANDLING(ERROR_DUPLICATE_BATCH_NUMBER, "duplicate batch number", "número de lote duplicado")
        ERROR_CASE_HANDLING(ERROR_INVALID_NAME, "invalid name", "nome inválido")
        ERROR_CASE_HANDLING(ERROR_INVALID_BATCH, "invalid batch", "lote inválido")
        ERROR_CASE_HANDLING(ERROR_INVALID_DATE, "invalid date", "data inválida")
        ERROR_CASE_HANDLING(ERROR_INVALID_QUANTITY, "invalid quantity", "quantidade inválida")
        ERROR_CASE_HANDLING(ERROR_NO_STOCK, "no stock", "esgotado")
        ERROR_CASE_HANDLING(ERROR_ALREADY_VACCINATED, "already vaccinated", "já vacinado")
        ERROR_CASE_HANDLING(ERROR_NO_MEMORY, "no memory", "sem memória")

        case ERROR_NO_SUCH_VACCINE:
            if (estate->pt_mode) printf("%s: vacina inexistente\n", vaccine_name);
            else                 printf("%s: no such vaccine\n", vaccine_name);
            return;

        case ERROR_NO_SUCH_BATCH:
            if (estate->pt_mode) printf("%s: lote inexistente\n", batch_name);
            else                 printf("%s: no such batch\n", batch_name);
            return;

        case ERROR_NO_SUCH_USER:
            if (estate->pt_mode) printf("%s: utente inexistente\n", utente_name);
            else                 printf("%s: no such user\n", utente_name);
            return;
    }

    __builtin_unreachable();
}

void set_error(Error error, ErrorState* estate, char* batch_name, char* vaccine_name, char* utente_name) {

    // função para setar um erro, e imprimir a mensagem de erro correspondente

    estate->error_emitted = true;
    estate->return_code = (int)error;

    print_error_string(error, estate, batch_name,  vaccine_name, utente_name);
}

void internal_error(const char* error_msg) {

    // erro interno, serve para tratar erros que não deveriam acontecer, se 
    // acontecerem, é um erro de programação

    printf("INTERNAL ERROR: %s\n", error_msg);
    abort();
}
