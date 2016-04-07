/*
 * JOOS is Copyright (C) 1997 Laurie Hendren & Michael I. Schwartzbach
 *
 * Reproduction of all or part of this software is permitted for
 * educational or research use on condition that this copyright notice is
 * included in any copy. This software comes with no warranty of any
 * kind. In no event will the authors be liable for any damages resulting from
 * use of this software.
 *
 * email: hendren@cs.mcgill.ca, mis@brics.dk
 */
#include <stdlib.h>

enum{
    INT,
    STRING,
    SHORT,
    CHAR,
    NIL,
    OBJECT
}val_type;

typedef union value{
  int integer;
  short short_int;
  char * string;
  char character;
  void * null;
  void * object;
}val;

typedef union entry{
  struct stack_entry{
    int stack_type;
    val stack_value;
  }stack_entry;
}entry;

typedef struct stack_table{
  entry ** list;
  CODE ** start_ptr;
  int top_index;
  int cap;
}stk_tbl;

stk_tbl joos_stack_table;
int init = 0;

void init_stack_tbl(CODE ** c){
  joos_stack_table.list = calloc(512, sizeof(entry *));
  joos_stack_table.cap = 512;
  joos_stack_table.top_index = 0;
  joos_stack_table.start_ptr = c;
}

void push(entry * add){
  if(joos_stack_table.cap == joos_stack_table.top_index){
    joos_stack_table.list = realloc(joos_stack_table.list, joos_stack_table.cap * 2);
    joos_stack_table.cap *= 2;
  }
  joos_stack_table.list[joos_stack_table.top_index++] = add;
}

entry * pop(){
  return joos_stack_table.list[joos_stack_table.top_index--];
}

void pop_all(){
  for(int i = 0; i < joos_stack_table.top_index; i++)
    free(joos_stack_table.list[i]);
  joos_stack_table.top_index = 0;
}


/* iload x        iload x        iload x
 * ldc 0          ldc 1          ldc 2
 * imul           imul           imul
 * ------>        ------>        ------>
 * ldc 0          iload x        iload x
 *                               dup
 *                               iadd
 */

int simplify_multiplication_right(CODE **c)
{ int x,k;
  if (is_iload(*c,&x) &&
      is_ldc_int(next(*c),&k) &&
      is_imul(next(next(*c)))) {
     if (k==0) return replace(c,3,makeCODEldc_int(0,NULL));
     else if (k==1) return replace(c,3,makeCODEiload(x,NULL));
     else if (k==2) return replace(c,3,makeCODEiload(x,
                                       makeCODEdup(
                                       makeCODEiadd(NULL))));
     return 0;
  }
  return 0;

}

/* dup
 * astore x
 * pop
 * -------->
 * astore x
 */
int simplify_astore(CODE **c)
{ int x;
  if (is_dup(*c) &&
      is_astore(next(*c),&x) &&
      is_pop(next(next(*c)))) {
     return replace(c,3,makeCODEastore(x,NULL));
  }
  return 0;
}

int simplify_primitive_operation(CODE * c){
  int x;
  if(is_ldc_int(){

  }else if(is_ldc_int(*c, &x)){
    if(is_iadd(next(*c), &x) || is_isub(next(*c), &x)){
      if(x == 0)
        return replace(c, 2, NULL);
    }else if(is_imul(next(*c), &x) || is_idiv(next(*c), &x) || is_imod(next(*c), &x)){
      if(x == 1)
        return replace(c, 2, makeCODEldc_int(x, NULL));
    }
  }
}

/*
* e.g
* ldc x
* iadd
* ldc y
* iadd
* ->
* ldc x+y
* iadd
*/
int simplify_const_chain_ops(CODE * c){
  int x;
  int y;
  if(is_ldc_int(*c, &x) && is_ldc_int(next(next(*c)), &x)){
    if(is_iadd(next(*c)) && is_iadd(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(x + y, makeCODEiadd(NULL)));
    }else if(is_isub(next(*c)) && is_isub(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(x + y, makeCODEisub(NULL)));
    }else if(is_imul(next(*c)) && is_imul(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(x * y, makeCODEimul(NULL)));
    }else if(is_idiv(next(*c)) && is_idiv(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(x * y, makeCODEidiv(NULL)));
    }else if(is_iadd(next(*c)) && is_isub(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(x - y, makeCODEiadd(NULL)));
    }else if(is_isub(next(*c)) && is_iadd(next(next(next(*c))))){
      return replace(c, 2, makeCODEldc_int(-x + y, makeCODEiadd(NULL)));
    }
  }
  return 0;
}



/* iload x
 * ldc k   (0<=k<=127)
 * iadd
 * istore x
 * --------->
 * iinc x k
 */
int positive_increment(CODE **c)
{ int x,y,k;
  if(is_iinc(*c, &x, &y)){
  	if(y == 0)
		return kill_line(c);
  }
  else if (is_iload(*c,&x) &&
      is_ldc_int(next(*c),&k) &&
      is_iadd(next(next(*c))) &&
      is_istore(next(next(next(*c))),&y) &&
      x==y && 0<=k && k<=127) {
     return replace(c,4,makeCODEiinc(x,k,NULL));
  }else if(is_ldc_int(*c,&k) &&
      is_iload(next(*c),&x)  &&
      is_iadd(next(next(*c))) &&
      is_istore(next(next(next(*c))),&y){
     return replace(c,4,makeCODEiinc(x,k,NULL));
  }
  return 0;
}
/*
* So
* swap
* swap
* is pointless so we remove it
*/
int simplify_swap(CODE **c){
	if(is_swap(*c) && is_swap(next(*c))){
    return replace_modified(c, 2, NULL);
  }
  return 0;
}

int simplify_pop(CODE **c){
	char ** temp;
  int x;
	if((is_iload(*c,&x)       ||
		is_aload(*c,&x)         ||
		is_iload(*c,&x)         ||
		is_iload(*c,&x)         ||
		is_ldc_int(*c,&x)       ||
		is_ldc_string(*c, temp) ||
		is_simplepush(*c)       ||
		is_dup(*c)              ||
		is_new(*c, temp))       &&
		is_pop(next(*c)) ){
			return replace_modified(c,2,NULL);
		}
	return 0;
}
int simplify_load_store(CODE **c){
	int x,y;
	if(is_aload(*c,&x) &&
	   is_astore(next(*c),&y)){
    if(x == y)
		  return replace_modified(c,2,NULL);
	}
	else if(is_iload(*c,&x) &&
	   is_istore(next(*c),&y)) {
    if(x == y)
		  return replace_modified(c,2,NULL);
	}else if(is_astore(*c, &x) &&
           is_aload(next(*c), &y)){
    if(x == y)
      return replace_modified(c,2,NULL);
  }else if(is_istore(*c, &x) &&
           is_iload(next(*c), &y)){
    if(x == y)
      return replace_modified(c,2,NULL);
  }
	return 0;
}

int simplify_field(CODE ** c){
  char ** arg0;
  char ** arg1;
  if(is_getfield(*c, arg0) && is_putfield(next(*c), arg1)){
    for(char * i = arg1[0]; i != NULL; i = i + sizeof(char *)){

    }
  }else if(is_putfield(*c, arg0) && is_getfield(next(*c), arg1)){

  }
}

/* goto L1
 * ...
 * L1:
 * goto L2
 * ...
 * L2:
 * --------->
 * goto L2
 * ...
 * L1:    (reference count reduced by 1)
 * goto L2
 * ...
 * L2:    (reference count increased by 1)
 */
int simplify_goto_goto(CODE **c)
{ int l1,l2;
  if (is_goto(*c,&l1) && is_goto(next(destination(l1)),&l2) && l1>l2) {
     droplabel(l1);
     copylabel(l2);
     return replace(c,1,makeCODEgoto(l2,NULL));
  }
  return 0;
}

int simplify_nop(CODE **c){
	if(is_nop(*c)){
    return replace_modified(c, 1, NULL);
	}
	return 0;
}

#define OPTS 4

OPTI optimization[OPTS] = {simplify_multiplication_right,
                           simplify_astore,
                           positive_increment,
                           simplify_goto_goto};
