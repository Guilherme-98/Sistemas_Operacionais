#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef struct entradaTLB {
	unsigned char endLogico;
	unsigned char endFisico;
}EntradaTLB;

static const unsigned char* nomeArquivo = "teste.dat";
static FILE* arquivo;
static FILE* arquivoEnderecos;
static EntradaTLB tlb[16];
static int numeroEntradasTLB = 0;
static int contadorTotalEnd = 0;
static int contadorTlbHit = 0;
static int contadorPageFault = 0;
static int tabelaPag[256];
static int indexTabelaPag[128];
static unsigned char paginaLivre = 0;
static char* backing;
static int swap = 0;
static unsigned char memoria[32768];

void lendoArquivoEnd();
int buscarTLB(unsigned char pagLogica);
int maiorNumero(int a, int b);

int main(int argc, unsigned char** argv) {

    //Checando Argumentos
    if(argc != 2) {
		printf("Incorrect number of parameters.\nUse: ./a.out fileNameAddresses\n");
		exit(0);
	}

   //Abrindo Arquivos
    arquivo = fopen(nomeArquivo, "r");
	arquivoEnderecos = fopen(argv[1], "r");
	if(arquivoEnderecos == NULL) {
		printf("The file %s was not found.\n", argv[1]);
		exit(0);
	}

    //Preenchendo Tabela Paginas
    int i;
	for(i = 0; i < 256; i++) {
		tabelaPag[i] = -1;
	}

	lendoArquivoEnd();

    //Imprimindo Estatisticas
    printf("Number of Translated Addresses = %d\n", contadorTotalEnd);
	printf("Page Faults = %d\n", contadorPageFault);
	printf("Page Fault Rate = %.3f\n", (1.* contadorPageFault / contadorTotalEnd));
	printf("TLB Hits = %d\n", contadorTlbHit);
	printf("TLB Hit Rate = %.3f\n", (1.* contadorTlbHit / contadorTotalEnd));

    //Fechando Arquivos
	fclose(arquivo);
	fclose(arquivoEnderecos);
	return 0;
}

void lendoArquivoEnd() {
	unsigned char buffer[10];
	int endLogico;
	int endFisico;
	int offset;
	int pagLogica;
	int pagFisica;
	int valor;
	backing = mmap(0, 65536, 0x1, 0x02, fileno(arquivo), 0);
	while (fgets(buffer, 10, arquivoEnderecos) != NULL) {
		endLogico = atoi(buffer);
		offset = endLogico & 255;
		pagLogica = (endLogico >> 8) & 255;
		pagFisica = buscarTLB(pagLogica);
		if(pagFisica != -1) {
			contadorTlbHit++;
		} else {
			pagFisica = tabelaPag[pagLogica];
			if(pagFisica == -1) {
				contadorPageFault++;
				int indice;
				if(paginaLivre < 128) {
					indice = paginaLivre;
					paginaLivre++;
				} else{
					paginaLivre = 0;
					indice = paginaLivre;
					paginaLivre++;
					swap = 1;
				}
				pagFisica = indice;
				if(swap) {
					tabelaPag[indexTabelaPag[pagFisica]] = -1;
				}
				memcpy(memoria + pagFisica * 256, backing + pagLogica * 256, 256);
				indexTabelaPag[pagFisica] = pagLogica;
				tabelaPag[pagLogica] = pagFisica;
			}
			EntradaTLB * entrada = & tlb[numeroEntradasTLB % 16];
			numeroEntradasTLB++;
			entrada->endLogico = pagLogica;
			entrada->endFisico = pagFisica;
		}
		
		endFisico = (pagFisica << 8) | offset;
		valor = memoria[pagFisica * 256 + offset];
		printf("Virtual address: %d Physical address: %d Value: %d\n", endLogico, endFisico, valor);
		contadorTotalEnd++;
	}
}

int buscarTLB(unsigned char pagLogica) {
	int i;
	for (i = maiorNumero((numeroEntradasTLB - 16), 0); i < numeroEntradasTLB; i++) {
		EntradaTLB * entrada = & tlb[i % 16];
		if (entrada->endLogico == pagLogica) {
			return entrada->endFisico;
		}
	}
	return -1;
}

int maiorNumero(int a, int b) {
	return a > b ? a : b;
}