#include <stdio.h>
#include "meu_banco.h"

int main(void)
{
    int op;
    do {
        if (bancoAberto) alertaSaldoNegativo();
        printf("\n===== BANCO DIGITAL =====\n1 - Abrir banco\n2 - Consultar historico\n3 - Efetuar sorteio\n4 - Sair\n");
        op = lerInt("Opcao: ");

        if ((op == 2 || op == 3) && !bancoAberto) { printf("Banco fechado!\n"); continue; }

        switch (op) {
            case 1: abrirBanco(); break;
            case 2: consultarHistorico(); break;
            case 3: efetuarSorteio(); break;
            case 4: liberarMemoria(); printf("Sistema encerrado.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (op != 4);
    return 0;
}