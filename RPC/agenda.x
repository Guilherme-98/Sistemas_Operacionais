/* agenda.x : definição da interface */
 
#define PROGRAM_NUMBER 12345678
#define VERSION_NUMBER 1
  
typedef string str_t<255>;
typedef struct contato_t *contato_p;
 
struct contato_t {
  str_t nome;
  str_t endereco;
  str_t telefone;
}; 


union consulta_res switch (int err) {
	case 0:
		contato_p contato;
	default:
		void;
};

typedef contato_t contatos_t<32>;
 
/* Interface que será oferecida aos clientes conectados*/
 
program AGENDA_PROG
{
  version AGENDA_VERSION 
  {
    consulta_res CONSULTA (str_t) = 1;
    int INSERE (struct contato_t) = 2;
    int ALTERA (struct contato_t) = 3;
    int REMOVE (str_t)            = 4;
    contatos_t LISTA ()             = 5;
  }
  = VERSION_NUMBER;
}
= PROGRAM_NUMBER;
