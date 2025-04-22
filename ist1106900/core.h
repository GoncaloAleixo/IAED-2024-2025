/**
 * @file core.h
 * @author ist1106900 (Goncalo Aleixo)
*/

#pragma once

/// \brief Definições de tipos e constantes para o projeto

/* -------------------------------------------------------------------------- */
/*                                  type defs                                 */
/* -------------------------------------------------------------------------- */

// bool impl

#ifdef bool
#undef bool
#endif

#ifdef true
#undef true
#endif

#ifdef false
#undef false
#endif

/// \brief Tipo booleano
typedef _Bool bool;
/// \brief Valor verdadeiro
#define true 1
/// \brief Valor falso
#define false 0


// numeric types

/// \brief Tipo inteiro assinado de 8 bits
typedef char i8;
/// \brief Tipo inteiro assinado de 16 bits
typedef short i16;
/// \brief Tipo inteiro assinado de 32 bits
typedef int i32;
/// \brief Tipo inteiro assinado de 64 bits
typedef long long i64;

/// \brief Tipo inteiro não assinado de 8 bits
typedef unsigned char u8;
/// \brief Tipo inteiro não assinado de 16 bits
typedef unsigned short u16;
/// \brief Tipo inteiro não assinado de 32 bits
typedef unsigned int u32;
/// \brief Tipo inteiro não assinado de 64 bits
typedef unsigned long long u64;

/// \brief Tipo numérico de ponto flutuante de 32 bits
typedef float f32;
/// \brief Tipo numérico de ponto flutuante de 64 bits
typedef double f64;

/// \brief Tipo inteiro não assinado do tamanho de um ponteiro
typedef unsigned long usize;

// numeric limits
/// \brief Valor máximo de um inteiro não assinado de 32 bits
#define U32_MAX ((u32)-1)
/// \brief Valor máximo de um inteiro não assinado do tamanho de um ponteiro
#define USIZE_MAX ((usize)-1)



/* -------------------------------------------------------------------------- */
/*                            application constants                           */
/* -------------------------------------------------------------------------- */

/// \brief Data de início do programa
#define START_DATE_DAY 1
/// \brief Mês de início do programa
#define START_DATE_MONTH 1
/// \brief Ano de início do programa
#define START_DATE_YEAR 2025
/// \brief Passo de crescimento do buffer de entrada
#define INPUT_BUFFER_STEP_SIZE 1024
/// \brief Tamanho máximo do nome de uma vacina
#define VACCINE_NAME_MAX_LENGTH 50
/// \brief Tamanho máximo do nome de um lote
#define BATCH_NAME_MAX_LENGTH 20
/// \brief número máximo de vacinas
#define MAX_VACCINE_NUMBER 1000

// data types settings
/// \brief Fator de crescimento do vetor
#define VECTOR_GROWTH_FACTOR 2
/// \brief Quantidade inicial de buckets de um hashmap
#define NAME_HASHMAP_INIT_BUCKETS 2
/// \brief Fator de crescimento do hashmap
#define NAME_HASHMAP_GROW_FACTOR 2
/// \brief Fator de carga máximo do hashmap
/// \note O fator de carga máximo é o número máximo de entradas dividido pelo número de buckets
#define NAME_HASHMAP_MAX_LOAD_FACTOR 1.0


/* -------------------------------------------------------------------------- */
/*                               error handling                               */
/* -------------------------------------------------------------------------- */

/// \enum Error
/// \brief Enumeração de erros possíveis
typedef enum {
    ERROR_TOO_MANY_VACCINES = 2,
    ERROR_DUPLICATE_BATCH_NUMBER,
    ERROR_INVALID_NAME,
    ERROR_INVALID_BATCH,
    ERROR_INVALID_DATE,
    ERROR_INVALID_QUANTITY,
    ERROR_NO_SUCH_VACCINE,
    ERROR_NO_STOCK,
    ERROR_ALREADY_VACCINATED,
    ERROR_NO_SUCH_BATCH,
    ERROR_NO_SUCH_USER,
    ERROR_NO_MEMORY,
} Error;

/// \class ErrorState
/// \brief Estrutura que representa o estado de erro da aplicação
typedef struct {
    bool error_emitted; //!< Indica se um erro foi emitido
    bool error_internal; //!< Indica se o erro é interno
    bool pt_mode; //!< Indica se o modo de erro é em português
    int return_code; //!< Código de retorno do erro
    char cmd_ch; //!< Comando que causou o erro
} ErrorState;

/// \public \memberof ErrorState
/// \brief Indica a ocorrência de um erro
/// \param error Código do erro
/// \param estate Ponteiro para o estado de erro
/// \param batch_name Nome do lote, opcional.
/// \param vaccine_name Nome da vacina, opcional.
/// \param utente_name Nome do utente, opcional.
void set_error(Error error, ErrorState* estate, char* batch_name, char* vaccine_name, char* utente_name);

/// \brief Lança um erro interno
/// \param error_msg Mensagem de erro
void internal_error(const char* error_msg);

#ifdef assert
#undef assert
#endif

/// \brief Etapa 2 da macro para transformar um número em string
#define STRINGIFY_(x) #x
/// \brief Etapa 1 da macro para transformar um número em string
#define STRINGIFY(x) STRINGIFY_(x)

/// \brief Macro para verificar uma condição e lançar um erro interno
/// \param x Condição a ser verificada
/// \param msg Mensagem de erro
#define assert(x, msg) if (!(x))  \
    internal_error( "expression failed: " #x "(" STRINGIFY(__LINE__) ", " __FILE__ "), " msg "." )

#ifndef NDEBUG
/// \brief Macro para verificar uma condição e lançar um erro interno em modo de depuração
/// \param x Condição a ser verificada
/// \param msg Mensagem de erro
#define debug_assert(x, msg) assert(x, msg)
#else
#define debug_assert(x, msg) ((void)0)
#endif
