/* Arquivo server.c: um servidor RPC simples */
#include <stdio.h>
#include "agenda.h"

typedef struct contato_bin{
   char nome[255];
   char endereco[255];
   char telefone[255];
} contato_bin;
 
/* implementação da função consulta */
consulta_res * consulta_1_svc (str_t *nome, struct svc_req *rqstp) {
   static consulta_res res;
 
   printf("Recebeu consulta: %s\n", *nome);
   
   xdr_free((xdrproc_t) xdr_consulta_res, &res);

   contato_p *cp = &res.consulta_res_u.contato;

   FILE * arquivo = fopen("dados.bin", "rb+");
   if (arquivo != NULL){
      contato_bin contato_aux;
      while(fread(&contato_aux, sizeof(contato_bin), 1, arquivo)){
         if(strcmp(*nome, contato_aux.nome) == 0){
            contato_p cont = *cp = malloc(sizeof(struct contato_t));

            res.err = 0;
            cont->nome = strdup(contato_aux.nome);
            cont->endereco = strdup(contato_aux.endereco);
            cont->telefone = strdup(contato_aux.telefone);

            fclose(arquivo);
            return (&res);
         }
      }
      fclose(arquivo);
   }

   *cp = NULL;
   res.err = 0;
   return (&res);
}

int * insere_1_svc(struct contato_t *contato, struct svc_req *rqstp) {
   static int res;

   printf("Recebeu inserção: %s, %s, %s\n", contato->nome, contato->endereco, contato->telefone);

   FILE * arquivo = fopen("dados.bin", "rb+");
   if (arquivo == NULL){
      arquivo = fopen("dados.bin", "wb+");
   } else{
      contato_bin contato_aux;
      while(fread(&contato_aux, sizeof(contato_bin), 1, arquivo)){
         if(strcmp(contato->nome, contato_aux.nome) == 0){
            res = 0;

            fclose(arquivo);
            return (&res);
         }
      }
   } 
   

   contato_bin contato_aux;
   strcpy(contato_aux.nome, contato->nome);
   strcpy(contato_aux.endereco, contato->endereco);
   strcpy(contato_aux.telefone, contato->telefone);

   fwrite(&contato_aux, sizeof(contato_bin), 1, arquivo);

   fclose(arquivo);
   res = 1;
   return (&res);
}

int * altera_1_svc(struct contato_t *contato, struct svc_req *rqstp) {
   static int res;
   int cont = 0;

   printf("Recebeu alteração: %s, %s, %s\n", contato->nome, contato->endereco, contato->telefone);

   FILE * arquivo = fopen("dados.bin", "rb+");
   if (arquivo != NULL){
      contato_bin contato_aux;
      while(fread(&contato_aux, sizeof(contato_bin), 1, arquivo)){
         if(strcmp(contato->nome, contato_aux.nome) == 0){
            contato_bin contato_aux2;
            strcpy(contato_aux2.nome, contato->nome);
            strcpy(contato_aux2.endereco, contato->endereco);
            strcpy(contato_aux2.telefone, contato->telefone);

            fseek(arquivo, sizeof(contato_bin) * cont, SEEK_SET);
            fwrite(&contato_aux2, sizeof(contato_bin), 1, arquivo);

            fclose(arquivo);
            res = 1;
            return (&res);
         }

         cont++;
      }

      fclose(arquivo);
   } 

   res = 0;
   return (&res);
}

int * remove_1_svc(str_t *nome, struct svc_req *rqstp) {
   static int res = 0;
   contato_bin* cb = malloc(sizeof(contato_bin));
   int cont = 0;

   printf("Recebeu remoção: %s\n", *nome);

   res = 0;
   FILE * arquivo = fopen("dados.bin", "rb");
   if (arquivo != NULL){
      while(fread(&cb[cont], sizeof(contato_bin), 1, arquivo)){
         if(strcmp(*nome, cb[cont].nome) != 0){
            cont++;
            cb = realloc(cb, (cont + 1) * sizeof(contato_bin));
         } else{
            res = 1;
         }
      }

      fclose(arquivo);
      FILE * arquivo = fopen("dados.bin", "wb");
      fwrite(cb, sizeof(contato_bin), cont, arquivo);
      fclose(arquivo);

      free(cb);
   }

   return (&res);
}

contatos_t * lista_1_svc(void *a, struct svc_req *rqstp) {
   static contatos_t res;

   printf("Recebeu listagem\n");

   xdr_free((xdrproc_t) xdr_contatos_t, &res);

   res.contatos_t_len = 0;
   res.contatos_t_val = NULL;

   contato_p contatos = NULL;
   int cont = 0;

   FILE * arquivo = fopen("dados.bin", "rb");
   if (arquivo != NULL){
      contato_bin contato_aux;
      while(fread(&contato_aux, sizeof(contato_bin), 1, arquivo)) {

         if (contatos == NULL) {
            contatos = malloc(sizeof(contato_t));
         } else {
            contatos = realloc(contatos, (cont + 1) * sizeof(contato_t));
         }

         contatos[cont].nome = strdup(contato_aux.nome);
         contatos[cont].endereco = strdup(contato_aux.endereco);
         contatos[cont].telefone = strdup(contato_aux.telefone);

         cont++;
      }

      fclose(arquivo);

      res.contatos_t_len = cont;
      res.contatos_t_val = contatos;
   }

   return (&res);
}
