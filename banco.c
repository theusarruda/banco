#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "meu_banco.h"

int bancoAberto = 0;
static Cliente *cl = NULL;
static int n = 0, seq = 1000;

int lerInt(const char *m)
{
    int v;
    printf("%s", m);
    while (scanf("%d", &v) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        if (c == EOF) exit(0);
        printf("Invalido. %s", m);
    }
    return v;
}

static double lerDouble(const char *m)
{
    double v;
    printf("%s", m);
    while (scanf("%lf", &v) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        if (c == EOF) exit(0);
        printf("Invalido. %s", m);
    }
    return v;
}

static int lerSimNao(const char *m)
{
    char c;
    printf("%s", m);
    while (scanf(" %c", &c) == 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        if (c == '1' || c == 's' || c == 'S') return 1;
        if (c == '0' || c == 'n' || c == 'N') return 0;
        printf("Invalido. %s", m);
    }
    exit(0);
}

static Conta *buscar(Cliente *c, int tipo)
{
    for (int i = 0; i < c->qtdContas; i++)
        if (c->contas[i].tipo == tipo) return &c->contas[i];
    return NULL;
}

static Conta *comTitulo(Cliente *c)
{
    for (int i = 0; i < c->qtdContas; i++)
        if (c->contas[i].titulo) return &c->contas[i];
    return NULL;
}

static double total(const Cliente *c)
{
    double t = 0;
    for (int i = 0; i < c->qtdContas; i++) t += c->contas[i].saldo;
    return t;
}

static void reg(Conta *k, const char *d, double v)
{
    k->hist = realloc(k->hist, (k->qtdHist + 1) * sizeof(Historico));
    if (!k->hist) exit(1);
    snprintf(k->hist[k->qtdHist].desc, sizeof(k->hist[0].desc), "%s", d);
    k->hist[k->qtdHist++].valor = v;
}

static void novaConta(Cliente *c, int tipo)
{
    c->contas = realloc(c->contas, (c->qtdContas + 1) * sizeof(Conta));
    if (!c->contas) exit(1);
    Conta *k = &c->contas[c->qtdContas++];
    snprintf(k->numero, sizeof(k->numero), "%d-%02d", seq++, tipo);
    k->tipo = tipo; k->saldo = 0; k->titulo = 0; k->hist = NULL; k->qtdHist = 0;
    reg(k, "Abertura de conta", 0);
}

static int aptoEmp(Cliente *c)
{
    Conta *p = buscar(c, 3);
    return buscar(c, 1) && p && p->saldo > 0 && !c->emprestimo;
}
static double limite(Cliente *c) { return 0.25 * 10 * total(c); }

static int aptoTitulo(Cliente *c)
{
    Conta *s = buscar(c, 2);
    return s && !s->titulo && s->saldo >= 1000 && s->saldo <= 1500;
}

/* ---------- Cadastro ---------- */
static void cadastrar(void)
{
    do n = lerInt("Quantos clientes serao cadastrados? "); while (n <= 0);
    cl = calloc(n, sizeof(Cliente));
    if (!cl) exit(1);

    for (int i = 0; i < n; i++) {
        Cliente *c = &cl[i];
        printf("\n--- Cliente %d ---\nNome: ", i + 1);
        scanf(" %49[^\n]", c->nome);
        do c->salario = lerDouble("Salario: R$ "); while (c->salario < 0);

        while (c->qtdContas == 0) {   /* pelo menos uma conta */
            for (int t = 1; t <= 3; t++) {
                if (t == 1 && buscar(c, 2)) continue;   /* corrente x salario */
                if (t == 2 && buscar(c, 1)) continue;
                printf("Abrir conta %s? ", t == 1 ? "Corrente" : t == 2 ? "Salario" : "Poupanca");
                if (lerSimNao("(s/n): ")) novaConta(c, t);
            }
            if (c->qtdContas == 0) printf("Abra ao menos uma conta.\n");
        }
    }
}

static Cliente *escolher(void)
{
    for (int i = 0; i < n; i++) printf("%d - %s\n", i + 1, cl[i].nome);
    int i = lerInt("Cliente: ") - 1;
    if (i < 0 || i >= n) { printf("Cliente invalido.\n"); return NULL; }
    return &cl[i];
}

static Conta *escolherConta(Cliente *c)
{
    Conta *k = buscar(c, lerInt("Conta (1-Corrente 2-Salario 3-Poupanca): "));
    if (!k) printf("Cliente nao possui essa conta.\n");
    return k;
}

static void menuOperacoes(void)
{
    int o;
    do {
        alertaSaldoNegativo();
        printf("\n1-Depositar 2-Sacar 3-Creditar salario 4-Emprestimo 5-Titulo 6-Extrato 0-Voltar\n");
        o = lerInt("Opcao: ");
        if (o < 0 || o > 6) { printf("Opcao invalida.\n"); continue; }
        if (o == 0) break;

        Cliente *c = escolher();
        if (!c) continue;
        Conta *k;
        double v;

        switch (o) {
        case 1: case 2:
            if (!(k = escolherConta(c))) break;
            v = lerDouble("Valor: R$ ");
            if (v <= 0 || (o == 2 && k->tipo != 1 && k->saldo < v)) { printf("Valor invalido ou saldo insuficiente.\n"); break; }
            k->saldo += (o == 1 ? v : -v);
            reg(k, o == 1 ? "Deposito" : "Saque", o == 1 ? v : -v);
            printf("Saldo: R$ %.2f\n", k->saldo);
            break;
        case 3:
            k = buscar(c, 2); if (!k) k = buscar(c, 1); if (!k) k = &c->contas[0];
            k->saldo += c->salario;
            reg(k, "Credito de salario", c->salario);
            printf("Salario de R$ %.2f creditado em %s.\n", c->salario, k->numero);
            break;
        case 4:
            if (!aptoEmp(c)) { printf("Nao apto: precisa de corrente + poupanca com saldo e so 1 emprestimo.\n"); break; }
            v = lerDouble("Valor do emprestimo: R$ ");
            if (v <= 0 || v > limite(c)) { printf("Invalido. Limite: R$ %.2f\n", limite(c)); break; }
            k = buscar(c, 1);
            k->saldo += v; c->emprestimo = 1;
            reg(k, "Emprestimo", v);
            printf("Emprestimo creditado em %s.\n", k->numero);
            break;
        case 5:
            if (!aptoTitulo(c)) { printf("Nao apto: saldo da conta salario deve estar entre R$ 1000 e R$ 1500.\n"); break; }
            k = buscar(c, 2); k->titulo = 1;
            reg(k, "Titulo de capitalizacao", 0);
            printf("Titulo adquirido.\n");
            break;
        case 6:
            for (int i = 0; i < c->qtdContas; i++) {
                printf("Conta %s | R$ %.2f\n", c->contas[i].numero, c->contas[i].saldo);
                for (int j = 0; j < c->contas[i].qtdHist; j++)
                    printf("  %s | R$ %.2f\n", c->contas[i].hist[j].desc, c->contas[i].hist[j].valor);
            }
            break;
        }
    } while (1);
}

void abrirBanco(void)
{
    if (!bancoAberto) { cadastrar(); bancoAberto = 1; printf("\nBanco aberto!\n"); }
    menuOperacoes();
}

void alertaSaldoNegativo(void)
{
    for (int i = 0; i < n; i++) {
        Conta *k = buscar(&cl[i], 1);
        if (k && k->saldo < 0)
            printf("ALERTA: %s esta com saldo negativo na conta corrente (R$ %.2f)\n", cl[i].nome, k->saldo);
    }
}

void consultarHistorico(void)
{
    double t = 0;
    for (int i = 0; i < n; i++) t += total(&cl[i]);
    printf("\n== Saldo Total do banco ==\n%d | R$ %.2f\n", n, t);

    printf("== Clientes Aptos a Emprestimos ==\n");
    for (int i = 0; i < n; i++)
        if (aptoEmp(&cl[i]))
            printf("%s|%s|%.2f\n", cl[i].nome, buscar(&cl[i], 1)->numero, limite(&cl[i]));

    printf("== Clientes com Saldo Negativo na Conta Corrente ==\n");
    for (int i = 0; i < n; i++) {
        Conta *k = buscar(&cl[i], 1);
        if (k && k->saldo < 0) printf("%s|%s|%.2f\n", cl[i].nome, k->numero, k->saldo);
    }

    printf("== Clientes Aptos a Compra de Titulos ==\n");
    for (int i = 0; i < n; i++)
        if (aptoTitulo(&cl[i])) {
            Conta *s = buscar(&cl[i], 2);
            printf("%s|%s|%.2f\n", cl[i].nome, s->numero, s->saldo);
        }

    printf("== Clientes Contemplado no Sorteio ==\n");
    for (int i = 0; i < n; i++)
        if (cl[i].premio > 0)
            printf("%s|%s|%.2f\n", cl[i].nome, comTitulo(&cl[i])->numero, cl[i].premio);
}

void efetuarSorteio(void)
{
    if (n < 5) { printf("Minimo de 5 clientes.\n"); return; }

    int ord[n];
    for (int i = 0; i < n; i++) ord[i] = i;
    for (int i = 0; i < 5; i++) {              /* 5 maiores saldos */
        int m = i;
        for (int j = i + 1; j < n; j++)
            if (total(&cl[ord[j]]) > total(&cl[ord[m]])) m = j;
        int a = ord[i]; ord[i] = ord[m]; ord[m] = a;
    }
    for (int i = 0; i < 5; i++)
        if (!comTitulo(&cl[ord[i]])) {
            printf("Sorteio cancelado: %s esta entre os 5 maiores saldos e nao tem titulo.\n", cl[ord[i]].nome);
            return;
        }

    int peso[5] = {8, 5, 3, 2, 1};
    static int semente = 0;
    if (!semente) { srand((unsigned)time(NULL)); semente = 1; }
    int r = rand() % 19, acc = 0, v = 0;
    for (int i = 0; i < 5; i++) {
        acc += peso[i];
        if (r < acc) { v = ord[i]; break; }
    }

    Conta *k = comTitulo(&cl[v]);
    double premio = 10 * k->saldo;
    k->saldo += premio;
    cl[v].premio = premio;
    reg(k, "Premio do sorteio", premio);
    printf("\nVencedor: %s | Premio: R$ %.2f (conta %s)\n", cl[v].nome, premio, k->numero);
}

void liberarMemoria(void)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < cl[i].qtdContas; j++) free(cl[i].contas[j].hist);
        free(cl[i].contas);
    }
    free(cl); cl = NULL; n = 0;
}