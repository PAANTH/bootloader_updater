/*
 * global_vars.c
 *
 *  Created on: Dec 10, 2018
 *      Author: paanth
 */

#include "global_vars.h"
#include "string.h"
glob_vars_t g_vars;
void init_global_vars(){
	memset(&g_vars,0,sizeof(glob_vars_t));
}
