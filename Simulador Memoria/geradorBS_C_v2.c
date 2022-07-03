#include <stdio.h>
#include <stdlib.h>

void GerarArquivo();
void ConsultarArquivo();

void main()
{
//   GerarArquivo();  // para manter o mesmo aquivo, executar uma unica vez a geração do arquivo

	     
	ConsultarArquivo();
    

}
void GerarArquivo()
{   FILE * arq;

    
    if ((arq = fopen("teste.dat","wb")) == NULL){
       printf("Error! opening file");
       exit(1);
   }
    int i;
    unsigned char num;
    int numint;

    for (i=0;i<65536;i++)
    {   
        numint=rand();

        num=(char)(numint%256);

        fwrite(&num,sizeof(num),1,arq);
    }
    fclose(arq);
}

void ConsultarArquivo()
{   FILE * arq;
    int i,j;
    unsigned char num;
    int numint;
    long offset=0;
    if ((arq = fopen("teste.dat","rb")) == NULL){
       printf("Error! opening file");
       // Program exits if the file pointer returns NULL.
       exit(1);
   }
   

    for (i=0;i<65536;i++)
    {
        fread(&num, sizeof(num), 1, arq); 
        printf(">>>Endereço: %d >>> conteúdo: %d\n",i,num);
       
    }

    fclose(arq);
    
}

