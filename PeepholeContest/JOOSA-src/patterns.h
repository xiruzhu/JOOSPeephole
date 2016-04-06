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
  }
  return 0;
}
int useless_swap(CODE **c){
	if
}
int useless_pop(CODE **c)
{
	char ** next;
	if((is_iload(*c,&x)         || 
		is_aload(*c,&x)         ||
		is_iload(*c,&x)         ||
		is_iload(*c,&x)         ||
		is_ldc_int(*c,&x)       ||
		is_ldc_string(*c, next) ||
		is_simplepush(*c)       ||
		is_dup(*c)              ||
		is_new(*c, next))       &&
		is_pop(next(*c))){
			return replace_modified(c,2,NULL);
		}
	return 0;
}
int useless_load_store(CODE **c)
{
	int x,y
	if(is_aload(*c,&x) &&
	   is_astore(next(*c),&y) &&
	   x == y){
		return replace_modified(c,2,NULL);
	}
	if(is_iload(*c,&x) &&
	   is_istore(next(*c),&y) &&
	   x == y){
		return replace_modified(c,2,NULL);
	}
	return 0;
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
	if(is_swap(*c) && 
	   is_swap(next(*c))){
		return replace(c,1,makeCODEgoto(l2,NULL));
	}
	return 0;
}

#define OPTS 4

OPTI optimization[OPTS] = {simplify_multiplication_right,
                           simplify_astore,
                           positive_increment,
                           simplify_goto_goto};
