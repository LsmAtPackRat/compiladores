#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comp_grammar.h"
#include "comp_dict.h"
#include "iks_types.h"

static inline void __comp_grammar_symbol_init(comp_grammar_symbol_t *grammar_symbol) {
    grammar_symbol->token_type = IKS_SIMBOLO_INDEFINIDO;
    grammar_symbol->code_line_number = 0;
    grammar_symbol->value = NULL;
    grammar_symbol->iks_type = IKS_NOTYPE;
}

comp_grammar_symbol_t *new_comp_grammar_symbol() {
    comp_grammar_symbol_t *grammar_symbol;
    grammar_symbol = malloc(sizeof(comp_grammar_symbol_t));
    __comp_grammar_symbol_init(grammar_symbol);
    return grammar_symbol;
}

void comp_grammar_symbol_delete(comp_grammar_symbol_t *grammar_symbol) {
    free(grammar_symbol->value);
    grammar_symbol->value = NULL;
    free(grammar_symbol);
    grammar_symbol = NULL;
}

void comp_grammar_symbol_set(comp_grammar_symbol_t *grammar_symbol, int token_type, int code_line_number, char *value) {
    grammar_symbol->token_type = token_type;
    grammar_symbol->code_line_number = code_line_number;
    grammar_symbol->value = value;
}

void comp_grammar_symbol_print(comp_grammar_symbol_t *grammar_symbol) {
    printf("%s\n",grammar_symbol->value);
}

void symbol_table_append(char *identifier, comp_grammar_symbol_t *symbol, comp_dict_t *symbol_table) {
    comp_dict_t *new_entry;
    new_entry = new_comp_dict();
    if (comp_dict_is_empty(symbol_table)) {
      symbol_table->item = new_comp_dict_item();
      comp_dict_item_set(symbol_table->item,identifier,(void *)symbol);
      symbol_table->next=new_entry;
      symbol_table->prev=new_entry;
      new_entry->next=symbol_table;
      new_entry->prev=symbol_table;
      //new_entry->item = new_comp_dict_item();
      //comp_grammar_symbol_t *symbol;
      //symbol = new_comp_grammar_symbol();
      //comp_dict_item_set(new_entry->item,NULL,(void *)symbol);
    }
    else {
      new_entry->item = new_comp_dict_item();
      comp_dict_item_set(new_entry->item,identifier,(void *)symbol);
      comp_dict_append(symbol_table,new_entry);
    }
}

void symbol_table_init() {
    //symbol_table = new_comp_dict();
    //comp_grammar_symbol_t *symbol;
    //symbol = new_comp_grammar_symbol();
    //symbol_table->item = new_comp_dict_item();
    //comp_dict_item_set(symbol_table->item,"empty",(void *)symbol);
}

void symbol_table_print(comp_dict_t *symbol_table) {
    //printf("imprimindo: %X\n",symbol_table);
    comp_dict_t *temp;
    temp = symbol_table;
    int i=0;
    do {
        if (temp->item) {
        if (temp->item->value) {
          comp_grammar_symbol_t *s;
          s = temp->item->value;
          printf("symbol: %s\n\ttoken_type: %d\n\tline: %d\n\tidentifier: %s\n\tsymbol_table: %X\n\tdecl_type: %d\n\tiks_size: %d\n\t iks_type: %d\n", \
            comp_dict_item_key_get(temp->item),\
            s->token_type,\
            s->code_line_number,\
            s->value,\
            s->symbol_table,\
            s->decl_type,\
            s->iks_size,\
            s->iks_type);
        }} 
        temp = temp->next;
    } while(temp != symbol_table);
    printf("\n");
}

comp_grammar_symbol_t *search_symbol_global(comp_grammar_symbol_t *symbol, comp_stack_t *scope) {
    comp_grammar_symbol_t *ret=NULL;
    comp_stack_t *it_scope;
    it_scope = scope;
    do {
      comp_dict_t *symbol_table;
      symbol_table = comp_stack_top(it_scope);
      //printf("global looking for %s at: %X\n",symbol->value,symbol_table);
      ret = search_symbol_local(symbol,symbol_table); 
      it_scope = it_scope->below;
    } while ((ret==NULL) && (it_scope != it_scope->below)); 
    if (ret==NULL) { //look at global
      ret = search_symbol_local(symbol,comp_stack_top(it_scope)); 
    }
    return ret;
}

comp_grammar_symbol_t *search_symbol_local(comp_grammar_symbol_t *symbol, comp_dict_t *symbol_table) {
    comp_grammar_symbol_t *ret =NULL;
    //printf("local looking at: %X\n",symbol_table);
    if (!comp_dict_is_empty(symbol_table)) {
      comp_dict_t *temp;
      temp = symbol_table;
      do {
          if (temp->item) {
          if (temp->item->value) {
            comp_grammar_symbol_t *s;
            s = temp->item->value;
            int diff = strcmp(symbol->value,s->value);
            //printf("%s == %s :%d\n",symbol->value,s->value,diff);
            if (!diff) {
              ret = s;
              break;
            }
          }}
          temp = temp->next;    
      } while(temp != symbol_table);
    }
    return ret;
}


int exist_symbol_local(comp_grammar_symbol_t *symbol, comp_dict_t *symbol_table) {
    int ret = 0;
    //printf("local looking at: %X\n",symbol_table);
    if (!comp_dict_is_empty(symbol_table)) {
      comp_dict_t *temp;
      temp = symbol_table;
      do {
          if (temp->item) {
          if (temp->item->value) {
            comp_grammar_symbol_t *s;
            s = temp->item->value;
            int diff = strcmp(symbol->value,s->value);
            //printf("%s == %s :%d\n",symbol->value,s->value,diff);
            if (!diff) {
              ret = 1;
              break;
            }
          }}
          temp = temp->next;    
      } while(temp != symbol_table);
    }
    return ret;
}


int decl_symbol(comp_grammar_symbol_t *s,int iks_type, int decl_type, void *symbol_table) {
  int ret=1;
  s->iks_type = iks_type;
  switch (iks_type) {
    case IKS_INT:
      s->iks_size=4;
      break;
    case IKS_FLOAT:
      s->iks_size=8;
      break;
    case IKS_BOOL:
      s->iks_size=1;
      break;
    case IKS_CHAR:
      s->iks_size=1;
      break;
    case IKS_STRING:
      s->iks_size=1;
      break;
  }
  s->decl_type = decl_type;
  s->symbol_table = (comp_dict_t*)symbol_table;
  if (!exist_symbol_local(s,s->symbol_table)) {
    symbol_table_append(s->value,s,s->symbol_table);
  }
  else {
    ret=0;
    fprintf(stderr,"line %d: identificador '%s' já declarado\n",s->code_line_number,s->value);
  }
  return ret;
}

int update_decl_symbol(comp_grammar_symbol_t *s,int any_type,comp_grammar_symbol_t *lit) {
  switch(any_type) {
    case IKS_DECL_VECTOR:
      s->decl_type=any_type;
      s->iks_size = s->iks_size * atoi(lit->value);
      break;
    case IKS_STRING:
      s->iks_size = strlen(lit->value);
      break;
  }
  return 0;
}

int symbol_is_decl_type(comp_grammar_symbol_t *s,int decl_type) {
  int ret=1;
  if (!(s->decl_type==decl_type)) {
    ret=0;
  }
  return ret;
}


int iks_error(comp_grammar_symbol_t *s, int error_type) {
  int ret=0;
  switch(error_type) {
    case IKS_ERROR_USE:
      if (s->decl_type==IKS_DECL_VAR) {
        fprintf(stderr,"line %d: identificador '%s' deve ser usado como variavel\n",s->code_line_number,s->value);      
        ret=IKS_ERROR_VARIABLE;
      }
      else if (s->decl_type==IKS_DECL_VECTOR) {
        fprintf(stderr,"line %d: identificador '%s' deve ser usado como vetor\n",s->code_line_number,s->value);      
        ret=IKS_ERROR_VECTOR;
      }
      else if (s->decl_type==IKS_DECL_FUNCTION) {
        fprintf(stderr,"line %d: identificador '%s' deve ser usado como funcao\n",s->code_line_number,s->value);      
        ret=IKS_ERROR_FUNCTION;
      }
      else {
        fprintf(stderr,"line %d: identificador '%s' ???????????\n",s->code_line_number,s->value);      
        ret=99999;

      }
      break;
  }
  return ret;

}

comp_function_list *comp_function_list_create(){
	return NULL;
}

comp_function_symbol *function_symbol_set(int token_type, int code_line_number, char *identifier){
	comp_function_symbol *function_symbol;
	function_symbol = malloc(sizeof(comp_function_symbol));
	function_symbol->token_type = token_type;
	function_symbol->code_line_number = code_line_number;
	function_symbol->params_number = 0;		
	return function_symbol;
}

comp_function_list *function_symbol_insert(comp_function_list *function_list, comp_function_symbol *function_symbol){
	comp_function_list *new_entry;
	comp_function_list *prev_entry = NULL;
	comp_function_list *aux_entry = function_list;
	
	new_entry = malloc(sizeof(comp_function_list));
	new_entry->function_symbol = function_symbol;
	
    while ((aux_entry != NULL) && (strcmp(aux_entry->function_symbol.identifier, function_symbol.identifier)<0)){ 
		prev_entry = aux_entry;
		aux_entry = aux_entry->next;
	}

   if(function_list == NULL){
            function_list = new_entry;
            new_entry->next = NULL;
            new_entry->prev = NULL;
       }
       else{
            if (prev_entry == NULL){ 
                   new_entry->next = function_list;
                   new_entry->prev = NULL;
                   function_list->prev = new_entry;
                   function_list = new_entry;
            }
            else{ 
                  if(aux_entry == NULL){ 
                        new_entry->next = NULL;
                        new_entry->prev = prev_entry;
                        prev_entry->next = new_entry;
                  }
                  else{
                        new_entry->next = aux_entry;
                        new_entry->prev = aux_entry->prev;
                        aux_entry->prev = new_entry;
                        prev_entry->next = new_entry;
                  }
            }
       }
}
	

comp_function_list *function_symbol_search(comp_function_list *function_list, char *identifier){
	comp_function_list *aux_entry = function_list;
	
    while (aux_entry != NULL){
		if (strcmp(aux_entry->function_symbol.identifier, function_symbol.identifier) == 0){	
			return aux_entry;
		}		
		aux_entry = aux_entry->next;
	}
}

void function_param_insert(comp_function_list *function_list, char *identifier, int param_type){
	function_list->function_symbol.params_type[function_list->function_symbol.params_number] = param_type;
	function_list->function_symbol.params_number = function_list->function_symbol.params_number + 1;
}

int function_param_check(comp_function_list *function_list, char *identifier, int param_type[]){
	comp_function_list *function_symbol;
	function_symbol = function_symbol_search(function_list, identifier){
		
	for(int i = 0; i <= function_symbol->params_number){
		if(function_symbol->params_type[i] != param_type[i]){
			printf("Error on line %d. Function \"%s\" parameters doesn't match with declaration\n", function_symbol->code_line_number, function_symbol->identifier);
		}
	}
}

