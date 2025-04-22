/**
 * @file main.c
 * @author ist1106900 (Goncalo Aleixo)
*/

// c stdlib
#include <stdio.h>
#include <string.h>

// local
#include "core.h"
#include "data_types.h"
#include "application.h"



bool read_line_from_stdin(Vector* buffer, ErrorState* estate) {

    // limpa o buffer, para evitar que dados antigos interfiram na leitura
    vector_clear(buffer);
    vector_resize(buffer, INPUT_BUFFER_STEP_SIZE + 1, NULL, estate);

    usize idx = 0; // índice do buffer de entrada
    while (true) {
        
        i32 read_size = 0; // número de bytes lidos na última chamada
        scanf("%" STRINGIFY(INPUT_BUFFER_STEP_SIZE) "[^\n]%n", (char*)buffer->data + idx, &read_size);
        idx += read_size;

        if (idx == 0 && feof(stdin))
            return false;

        if (read_size == INPUT_BUFFER_STEP_SIZE) {

            // buffer cheio, aumenta o tamanho do buffer
            if (!vector_resize(buffer, buffer->size + INPUT_BUFFER_STEP_SIZE, NULL, estate))
                return false;
            
            continue;
        }

        vector_resize(buffer, idx + 1, NULL, estate);
        ((char*)buffer->data)[idx] = '\0';
        break;
    }

    char final_ch = (char)getchar(); // próximo caractere após a leitura
    if (final_ch != '\n' && final_ch != EOF)
        ungetc(final_ch, stdin);

    return true;
}

void reset_error_state(ErrorState* estate) {
    estate->error_emitted = false;
    estate->error_internal = false;
    estate->return_code = 0;
}


void application_main_loop(Application* app, Vector* input_buffer, ErrorState* estate) {

    while (read_line_from_stdin(input_buffer, estate)) {

        // reseta o estado de erro antes de processar a próxima instrução, impedindo que erros anteriores interfiram
        // no processamento da instrução atual
        reset_error_state(estate);

        // faz o parsing da instrução, que deve ser escrita dentro de 'instr' caso seja válida
        // se a flag 'jump' for ativada, então a instrução foi e deve ser ignorada
        bool jump; // flag que indica se a instrução deve ser ignorada
        Instruction instr; // estrutura onde será armazenada a instrução lida
        char* input_buffer_ptr = (char*)vector_at(input_buffer, 0);
        if (!parse_instruction(input_buffer_ptr, vector_size(input_buffer) - 1, &instr, estate, &jump)) {
            if (!estate->error_emitted || estate->error_internal || estate->return_code == ERROR_NO_MEMORY)
                break; // interrompe a execução do programa em caso de erro de memória ou erro interno
            else
                continue;
        }

        // caso a instrução seja inválida, pula para a próxima iteração
        if (jump)
            continue;
            
        // executa a instrução e verifica se o programa deve ser encerrado
        bool exit = false; // flag que indica se o programa deve ser encerrado
        if (!handle_instruction(app, &instr, &exit, estate)) {
            if (!estate->error_emitted || estate->error_internal || estate->return_code == ERROR_NO_MEMORY)
                break; // interrompe a execução do programa em caso de erro de memória ou erro interno
            else
                continue;
        }

        // caso a instrução seja de saída, encerra o programa
        if (exit)
            break;
    }
}

int main(int argc, char* argv[]) {

    ErrorState estate = {false, false, false, 0, ' '}; // estado de erro da aplicação

    // verifica se o modo pt foi ativado, se sim, configura o sistema de erro para enviar mensagens em português
    if (argc >= 2 && strcmp(argv[1], "pt") == 0)
        estate.pt_mode = true;

    // aloca um buffer para armazenar a linha lida do stdin, considerando o tamanho máximo determinado
    Vector input_buffer; // buffer de entrada
    vector_init(&input_buffer, sizeof(char), NULL);
    
    // inicializa a aplicação principal
    Application app; // objeto da aplicação
    if (!application_init(&app, &estate)) {
        vector_free(&input_buffer);
        return estate.return_code;
    }
    
    // ciclo principal da aplicação, onde as instruções são lidas e processadas
    application_main_loop(&app, &input_buffer, &estate);

    // libera os recursos alocados pela aplicação
    application_free(&app);
    vector_free(&input_buffer);
    return estate.return_code;
}
