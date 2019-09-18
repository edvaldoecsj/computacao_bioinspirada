#include <bits/stdc++.h>

using namespace std;

typedef vector<int> Genotipo;

#define n_geracoes  50
#define n_elite     1
#define t_populacao 20
#define max_dist    50
#define p_mutacao   0.01
#define p_crossover 0.75

int n_cidades = 10;
int mapa[500][500];

// Gera um número aleatório entre 0 e 1
double escolhe_aleatorio() {

    random_device rd;
    default_random_engine gen(rd());
    uniform_real_distribution<double> unif(0, 1);
    return unif(gen);
}

//Calcula a distância do percurso de um genótipo
int calc_distancia(Genotipo gen) {

    int dist = 0;
    for (int i = 1; i < n_cidades; ++i) {
        int cidade_A = gen[i - 1];
        int cidade_B = gen[i];
        dist += mapa[cidade_A][cidade_B];
    }
    return dist + mapa[gen[n_cidades - 1]][1];
}

struct Solucao {
    
    double fitness;
    int dist;
    Genotipo gen;

    Solucao () : fitness(0) {}

    Solucao (Genotipo g) : gen(g) {
        
        dist = calc_distancia(this->gen);
        fitness = 1.0 / dist;
    }

    Solucao (Genotipo g, double f) : gen(g), fitness(f) {
        dist = calc_distancia(this->gen);
    }

    bool operator<(const Solucao& s_2) const {
        return dist < s_2.dist;
    }

    // Sequential constructive crossover operator (SCX)
    Solucao aplica_crossover(const Solucao& s_2) {
        
        vector<bool> disponivel(n_cidades + 1, true);
        Genotipo g_i;
        g_i.push_back(1);
        disponivel[1] = false;
        while(g_i.size() < n_cidades) {
            int atual = g_i.back();

            // Seleciona os nós legítimos dos cromossomos
            int leg_1 = -1, leg_2 = -1;
            for (int i = 0; i < n_cidades && !(leg_1 != -1 && leg_2 != -1); ++i) {
                if (this->gen[i] == atual && i < n_cidades - 1 &&
                    leg_1 == -1 && disponivel[this->gen[i + 1]])
                        leg_1 = this->gen[i + 1];
                if (s_2.gen[i] == atual && i < n_cidades - 1 &&
                    leg_2 == -1 && disponivel[s_2.gen[i + 1]])
                        leg_2 = s_2.gen[i + 1];
            }

            // Caso nenhum tenha sido encontrado, procura o
            // primeiro legítimo em {2, 3, ... , n_cidades}
            if (leg_1 == -1)
                for (int i = 2; i <= n_cidades; ++i)
                    if (disponivel[i]) {
                        leg_1 = i;
                        break;
                    }
            if (leg_2 == -1)
                for (int i = 2; i <= n_cidades; ++i)
                    if (disponivel[i]) {
                        leg_2 = i;
                        break;
                    }

            // Aceita o melhor nó (estratégia gulosa)
            if (mapa[atual][leg_1] < mapa[atual][leg_2]) {
                g_i.push_back(leg_1);
                disponivel[leg_1] = false;
            } else {
                g_i.push_back(leg_2);
                disponivel[leg_2] = false;
            }
        }
        Solucao filho(g_i);
        return filho;
    }

    // Cada gene tem 'p_mutacao' de chance de sofrer mutação
    void aplica_mutacao() {

        for (int i = 1; i < n_cidades; ++i) {
            double prob = escolhe_aleatorio();
            if (prob >= p_mutacao) continue;

            int gene_A = 1 + rand() % (n_cidades - 1);
            int gene_B = 1 + rand() % (n_cidades - 1);
            int aux = gen[gene_A];
            gen[gene_A] = gen[gene_B];
            gen[gene_B] = aux;
        }
        dist = calc_distancia(this->gen);
        fitness = 1.0 / dist;
    }
};

typedef vector<Solucao> Populacao;

Populacao cria_populacao() {

    Populacao pop;

    // Array com as cidades em ordem crescente
    Genotipo order(n_cidades);
    for (int cidade = 1; cidade <= n_cidades; ++cidade)
        order[cidade - 1] = cidade;

    // Cria a população inicial com genótipos que são
    // permutações do array ordenado (cidade 1 é sempre
    // a primeira)
    for (int i = 0; i < t_populacao; ++i) {
        random_shuffle(order.begin() + 1, order.end());
        pop.push_back(Solucao(order));
    }
    return pop;
}

void le_mapa() {

    // Inicia o mapa com distâncias entre as cidades
    // salvas em arquivo txt
    ifstream arquivo;
    
    arquivo.open("mapa.txt");
    if (!arquivo) {
        cout << "Incapaz de abrir arquivo\n";
        exit(1); // Termina com erro
    }

    arquivo >> n_cidades;

    for (int i = 1; i <= n_cidades; ++i)
        for (int j = 1, x; j <= n_cidades; ++j)
            arquivo >> mapa[i][j];
    arquivo.close();
}

void cria_mapa() {

    // Inicia o mapa com distâncias aleatórias entre as cidades
    for (int i = 0; i <= n_cidades; ++i)
        for (int j = 0; j <= n_cidades; ++j) {
            if (i == j) {
                mapa[i][j] = 0;
                continue;
            }
            int val = 1 + rand() % max_dist;
            mapa[i][j] = val;
        }
}

void normaliza(Populacao& pop) {

    double total = 0;
    for (int i = 0; i < pop.size(); ++i)
        total += pop[i].dist;
    for (int i = 0; i < pop.size(); ++i)
        pop[i].fitness = pop[i].dist * 1.0 / total;
}

Solucao escolhe_um(const Populacao& pop) {

    double prob = escolhe_aleatorio();
    int i = 0;
    for (; prob > 0.0 && i < pop.size(); ++i)
        prob -= pop[i].fitness;
    return pop[max(0, i - 1)];
}

void mostra_mapa() {

    printf("Matriz de custos (mapa)\n    ");
    for (int i = 0; i <= n_cidades; ++i) {
        for (int j = 0; j <= n_cidades; ++j) {
            if (!i && j == n_cidades) continue;
            if (!i) printf("%3d", j + 1);
            else {
                if(j) printf("%3d", mapa[i][j]);
                else printf("%3d|", i);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void mostra_solucao(Solucao sol) {

    for (int j = 0; j < n_cidades; ++j)
        printf("%s%3d", j ? " ->" : "", sol.gen[j]);
    printf("  .:.  dist = %d; fit = %lf\n", sol.dist, sol.fitness);
}

void mostra_pop(Populacao pop) {

    printf("Pop:\n ");
    for (int i = 0; i < min((int) pop.size(), 8); ++i)
        mostra_solucao(pop[i]);
}

int main() {
    
    srand (time(NULL));

    // le_mapa();
    cria_mapa();
    mostra_mapa();

    Populacao pop = cria_populacao();

    for (int g = 0; g < n_geracoes; ++g) {
        Populacao nova_pop;

        // Reprodução elitista
        sort(pop.begin(), pop.end());
        
        for (int i = 0; i < n_elite; ++i)
            nova_pop.push_back(pop[i]);

        normaliza(pop);
        // mostra_pop(pop);

        // Crossover
        while (nova_pop.size() < t_populacao) {
            double prob = escolhe_aleatorio();
            if (prob >= p_crossover) continue;

            Solucao s_1 = escolhe_um(pop);
            Solucao s_2 = escolhe_um(pop);
            Solucao s_3 = s_1.aplica_crossover(s_2);
            Populacao selecionados = { s_1, s_2, s_3 };
            sort(selecionados.begin(), selecionados.end());

            nova_pop.push_back(selecionados[0]);
            if (nova_pop.size() < t_populacao)
                nova_pop.push_back(selecionados[1]);
        }

        // Mutação
        for (int j = 0; j < nova_pop.size(); ++j)
            nova_pop[j].aplica_mutacao();
        pop = nova_pop;
    }

    sort(pop.begin(), pop.end());
    printf("\n\nMelhor caminho: \n");
    mostra_solucao(pop[0]);

    return 0;
}