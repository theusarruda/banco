#ifndef MEU_BANCO_H
#define MEU_BANCO_H

typedef struct { char desc[60]; double valor; } Historico;

typedef struct {
    char numero[10];
    int tipo;
    double saldo;
    int titulo;
    Historico *hist;
    int qtdHist;
} Conta;

typedef struct {
    char nome[50];
    double salario;
    Conta *contas;
    int qtdContas;
    int emprestimo;
    double premio;
} Cliente;

extern int bancoAberto;

int  lerInt(const char *msg);
void abrirBanco(void);
void consultarHistorico(void);
void efetuarSorteio(void);
void alertaSaldoNegativo(void);
void liberarMemoria(void);

#endif