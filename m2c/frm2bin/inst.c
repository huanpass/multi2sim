/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>

#include <arch/fermi/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "frm2bin.h"
#include "inst.h"
#include "inst-info.h"
#include "symbol.h"
#include "token.h"

/*
#include "stream.h"
#include "task.h"
*/


struct frm2bin_inst_t *frm2bin_inst_create(struct frm2bin_pred_t *pred, char *name, struct list_t *arg_list)
{
	struct frm2bin_inst_t *inst;
	struct frm2bin_inst_info_t *info;

	struct frm_arg_t *arg;
	struct frm_token_t *token;
	struct frm_mod_t *mod;

	char err_str[MAX_STRING_SIZE];

	int index;

	/* string list for the modifier in the instruction */
	struct list_t *str_mod_list;

	/* list of modifier, type frm_mod_t */
	struct list_t *mod_list;

	/* The name paramter passed in constains both inst-name and mod_list */
	char *inst_name;

	char *mod_name;

	/* Allocate */
	inst = xcalloc(1, sizeof(struct frm2bin_inst_t));

	/* Initialize */
	if (!arg_list)
		arg_list = list_create();
	inst->arg_list = arg_list;

	/* create a list of tokens in the instruction using "." delimiter */
	/* "." is the delimiter between inst-name and modifier, also */
	/* modifier between mofifier */
	str_mod_list = str_token_list_create(name, ".");

	/* extract the inst-name, it will be removed from the str_mod_list */
	inst_name = list_remove_at(str_mod_list, 0);

	/* create mod_list from the str_mod_list */
	mod_list = list_create();
	if (str_mod_list->count > 0)
	{
		LIST_FOR_EACH(str_mod_list, index)
		{
			mod_name = list_get(str_mod_list, index);
			/* check which modifier it belongs to, then create a
			 * mod object, this should be changed later */
			if (!strcmp(mod_name, "U32")
				|| !strcmp(mod_name, "S32"))
			{
				/* create a modifier object, then add it to
				 * the tail of the list */
				mod = frm_mod_create_data_width(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "U8") ||
				!strcmp(mod_name, "S8") ||
				!strcmp(mod_name, "U16") ||
				!strcmp(mod_name, "S16") ||
				!strcmp(mod_name, "64") ||
				!strcmp(mod_name, "128"))
			{
				/* create mod0_B_type modifier, add to list */
				mod = frm_mod_create_mod0_B_type(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "AND") ||
				!strcmp(mod_name, "OR") ||
				!strcmp(mod_name, "XOR"))
			{
				/* create logic modifier, add to list */
				mod = frm_mod_create_logic(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "LT") ||
				!strcmp(mod_name, "EQ") ||
				!strcmp(mod_name, "LE") ||
				!strcmp(mod_name, "GT") ||
				!strcmp(mod_name, "NE") ||
				!strcmp(mod_name, "GE"))
			{
				/* create comparison modifier, add to list */
				mod = frm_mod_create_comparison(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "FMA64") ||
				!strcmp(mod_name, "FMA32") ||
				!strcmp(mod_name, "XLU") ||
				!strcmp(mod_name, "ALU") ||
				!strcmp(mod_name, "AGU") ||
				!strcmp(mod_name, "SU") ||
				!strcmp(mod_name, "FU") ||
				!strcmp(mod_name, "FMUL"))
			{
				/* create comparison modifier, add to list */
				mod = frm_mod_create_offs_mod1_A_op(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "RN") ||
				!strcmp(mod_name, "RM") ||
				!strcmp(mod_name, "RP") ||
				!strcmp(mod_name, "RZ"))
			{
				/* create mod1_B_rnd modifier, add to list */
				mod = frm_mod_create_gen0_mod1_B_rnd(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "BREV"))
			{
				/* create bit reverse modifier, add to list */
				mod = frm_mod_create_brev(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "TRIG"))
			{
				/* create bit reverse modifier, add to list */
				mod = frm_mod_create_offs_mod1_A_trig(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "SAT"))
			{
				/* create mod0_D_sat modifier, add to list */
				mod = frm_mod_create_mod0_D_sat(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "FTZ") ||
				!strcmp(mod_name, "FMZ"))
			{
				/* create ftzfmz modifier, add to list */
				mod = frm_mod_create_mod0_D_ftzfmz(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "CC"))
			{
				/* create bit reverse modifier, add to list */
				mod = frm_mod_create_dst_cc(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "LMT"))
			{
				/* create tgt_lmt modifer, add to list */
				mod = frm_mod_create_tgt_lmt(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "U"))
			{
				/* create tgt_u modifier, add to list */
				mod = frm_mod_create_tgt_u(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "W"))
			{
				/* create tgt_u modifier, add to list */
				mod = frm_mod_create_mod0_A_w(mod_name);
				list_add(mod_list, mod);
			}
			else if (!strcmp(mod_name, "S"))
			{
				/* create mod0_C_s modifier, add to list */
				mod = frm_mod_create_mod0_C_s(mod_name);
				list_add(mod_list, mod);
			}
			else
			{
				/* unsupported modifier */
				snprintf(err_str, sizeof err_str,
					"Unsupported modifier: %s", mod_name);
				frm2bin_yyerror(err_str);
			}
		}
	}

	inst->mod_list = mod_list;

	/* free str_mod_list */
	str_token_list_free(str_mod_list);


	/* if no predicate, set it to -1 */
	if (pred == NULL)
	{
		inst->pred_num = -1;
	}
	else
	{
		inst->pred_num = pred->number;
	}


	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. For Fermi, the name passed in contains
	 * both the instructionname and modifiler list. so we use inst_name
	 * created before */
	snprintf(err_str, sizeof err_str, "invalid instruction: %s",
		inst_name);
	for (info = hash_table_get(frm2bin_inst_info_table, inst_name); info;
		info = info->next)
	{
		/* Check number of arguments */
		if (arg_list->count != info->token_list->count)
		{
			printf("invalid # of args\n");
			snprintf(err_str, sizeof err_str,
				"invalid number of arguments for %s \
				(%d given, %d expected)", name, 
				arg_list->count, info->token_list->count - 1);
			continue;
		}

		/* Check arguments */
		err_str[0] = '\0';
		LIST_FOR_EACH(arg_list, index)
		{
			/* Get actual argument */
			arg = list_get(arg_list, index);

			/* Get formal argument from instruction info */
			token = list_get(info->token_list, index);
			assert(token);

			/* Check that actual argument type is acceptable for
			 * token, right now this func always return success */
			if (!frm_token_is_arg_allowed(token, arg))
			{
				printf("invalid args type\n");
				snprintf(err_str, sizeof err_str,
					"invalid type for argument %d",
					index + 1);
				break;
			}
		}

		/* Error while processing arguments */
		if (err_str[0])
			continue;

		/* All checks passed, instruction identified correctly as
		 * that represented by 'info'. */
		break;
	}

	/* Error identifying instruction */
	if (!info)
	{
		frm2bin_yyerror(err_str);
		exit(1);
	}

	/* Initialize opcode */
	inst->info = info;
	inst->opcode = info->inst_info->opcode;

	free(inst_name);

	/* Return */
	return inst;
}

void frm2bin_inst_free(struct frm2bin_inst_t *inst)
{
	int index;

	struct frm_arg_t *arg;
	struct frm_mod_t *mod;

	/* Free all argument object in the argument list */
	LIST_FOR_EACH(inst->arg_list, index)
	{
		arg = list_get(inst->arg_list, index);
		frm_arg_free(arg);
	}

	/* Free all argument object in the argument list */
	LIST_FOR_EACH(inst->mod_list, index)
	{
		mod = list_get(inst->mod_list, index);
		frm_mod_free(mod);
	}

	/* Free argument list, modifier list and instruction object */
	list_free(inst->mod_list);
	list_free(inst->arg_list);
	free(inst);
}


void frm2bin_inst_dump(struct frm2bin_inst_t *inst, FILE *f)
{
	struct frm_arg_t *arg;
	unsigned int word;
	
	int i;
	int j;

	/* Dump instruction opcode */
	fprintf(f, "Instruction %s\n", inst->info->name);
	/*
	fprintf(f, "\tformat=%s, size=%d\n",
			str_map_value(&frm_inst_fmt_map, inst->info->inst_info->fmt), 
			inst->size);
	*/

	/* Dump arguments */
	LIST_FOR_EACH(inst->arg_list, i)
	{
		arg = list_get(inst->arg_list, i);
		fprintf(f, "\targ %d: ", i);
		frm_arg_dump(arg, f);
		fprintf(f, "\n");
	}

	/* Empty instruction bits */
	if (!inst->size)
		return;

	/* Print words */
	for (i = 0; i < inst->size / 4; i++)
	{
		word = *(int *) &inst->inst_bytes.bytes[i * 4];
		printf("\tword %d:  hex = { %08x },  bin = {", i, word);
		for (j = 0; j < 32; j++)
			printf("%s%d", j % 4 ? "" : " ",
				(word >> (31 - j)) & 1);
		printf(" }\n");
	}
}


void frm2bin_inst_gen(struct frm2bin_inst_t *inst)
{
	/* Fermi uses frm_inst_dword_t inst_bytes it's different from
	 * southern-island */
	union frm_inst_dword_t *inst_bytes;

	struct frm_inst_info_t *inst_info;
	struct frm2bin_inst_info_t *info;

	struct frm_arg_t *arg;
	struct frm_token_t *token;

	/* fermi has modifier, south-island doesn't */
	struct frm_mod_t *mod;

	int index;

	/* Initialize */
	inst_bytes = &inst->inst_bytes;
	info = inst->info;
	assert(info);
	inst_info = info->inst_info;

	/* By default, the instruction has the number of bytes specified by
	 * its format. 4-bit instructions could be extended later to 8 bits
	 * upon the presence of a literal constant. right now for fermi, all
	 * of them are 8 bytes */
	inst->size = inst_info->size;

	/* Instruction opcode */
	switch (inst_info->fmt)
	{

		/* encoding in [31:26], op in [18:16] */

	case FRM_FMT_FP_FFMA:

		/* [3:0]: 0000 */
		inst_bytes->general0.op0 = 0x0;

		/* [9:4]: all 0s, default value */
		inst_bytes->general0.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [48]: 0 */
		inst_bytes->general0.dst_cc = 0x0;

		/* [56:55]: 00, default value */
		inst_bytes->general0_mod1_B.rnd = 0x0;

		/* [57]: 0 */
		inst_bytes->general0_mod1_B._const0 = 0x0;

		/* [63:58]: 001100 */
		inst_bytes->general0.op1 = 0xc;

		break;


		/* encoding in [:], op in [] */
	case FRM_FMT_FP_FADD:

		inst_bytes->general0.op0 = 0x0;
		/* [4] = 0, default value for other bits */
		inst_bytes->general0.mod0 = 0x0;
		inst_bytes->general0.dst_cc = 0x0;
		/* [54:50] = 0, default value for other bits */
		inst_bytes->general0.mod1 = 0x0;
		inst_bytes->general0.op1 = 0x14;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		break;

	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_FP_FADD32I:
		
		inst_bytes->mimg.enc = 0x3c;
		inst_bytes->mimg.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:27], op in [26:22] */
	/*
	case FRM_FMT_FP_FCMP:

		inst_bytes->smrd.enc = 0x18;
		inst_bytes->smrd.op = inst_info->opcode;
		break;
	*/
	
	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_FP_FMUL:
		
		inst_bytes->ds.enc = 0x36;
		inst_bytes->ds.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:23], op in [22:16] */
	/*
	case FRM_FMT_FP_FMUL32I:
		
		inst_bytes->sopc.enc = 0x17e;
		inst_bytes->sopc.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:23], op in [15:8] */
	/*
	case FRM_FMT_FP_FSETP:

		inst_bytes->sop1.enc = 0x17d;
		inst_bytes->sop1.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:30], op in [29:23] */
	/*
	case FRM_FMT_FP_MUFU:

		inst_bytes->sop2.enc = 0x2;
		inst_bytes->sop2.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:23], op in [22:16] */
	/*
	case FRM_FMT_FP_DFMA:

		inst_bytes->sopp.enc = 0x17f;
		inst_bytes->sopp.op = inst_info->opcode;
		break;
	*/
	
	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_FP_DADD:
		
		inst_bytes->sopk.enc = 0xb;
		inst_bytes->sopk.op = inst_info->opcode;
		break;
	*/

	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_FP_DMUL:
		
		inst_bytes->vopc.enc = 0x3e;
		inst_bytes->vopc.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31:25], op in [16:9] */
	/*
	case FRM_FMT_FP_DSETP:

		inst_bytes->vop1.enc = 0x3f;
		inst_bytes->vop1.op = inst_info->opcode;
		break;
	*/

	/* encoding in [31], op in [30:25] */
	case FRM_FMT_INT_IMAD:

		inst_bytes->general0.op0 = 0x3;
		/* [4] = 0, others are default value */
		inst_bytes->general0.mod0 = 0x0;
		/* default value */
		inst_bytes->general0.dst_cc = 0x0;	/* only 5 bits here */
		/* [55]=0, [57]=0, [56] default */
		inst_bytes->general0.mod1 &= 0x1f;
		inst_bytes->general0.op1 = 0x8;


		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		break;

	case FRM_FMT_INT_IMUL:

		/* [3:0]: 0011 */
		inst_bytes->general0.op0 = 0x3;

		/* [9:4]: 001010, default value */
		inst_bytes->general0.mod0 = 0xa;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [47:46]: src2_mod, don't know the default value yet */

		/* [48]: 0 , dst.cc, default value */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: 000000000 */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 010100 */
		inst_bytes->general0.op1 = 0x14;

		break;

	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_INT_IADD:
		
		inst_bytes->vintrp.enc = 0x32;
		inst_bytes->vintrp.op = inst_info->opcode;
		break;
	*/

	/* encoding in [:], op in [] */
	/*
	case FRM_FMT_INT_IADD32I:
		
		inst_bytes->exp.enc = 0x3e;
		break;
	*/

	case FRM_FMT_INT_ISCADD:

		inst_bytes->general0.op0 = 0x3;
		/* [4] = 0 */
		inst_bytes->general0.mod0 &= 0xfffe;
		/* default value */
		inst_bytes->general0.dst_cc = 0x0;
		/* [54:49] = 0 [57] = 0, others default value */
		inst_bytes->general0.mod1 = 0x0;
		inst_bytes->general0.op1 = 0x10;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		break;

	case FRM_FMT_INT_BFE:

		/* [3:0]: 0011 */
		inst_bytes->general0.op0 = 0x3;

		/* [9:4]: 000010, [5], [8] are default value */
		inst_bytes->general0.mod0 = 0x2;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [47:46]: 00, (default value not sure) */
		inst_bytes->general0.src2_mod = 0x0;

		/* [48]: 0, dst.cc (default value) */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: 000000000 */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 011100 */
		inst_bytes->general0.op1 = 0x1c;

		break;

	//case FRM_FMT_INT_BFI:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	case FRM_FMT_INT_SHR:

		/* [3:0]: 0011 */
		inst_bytes->general0.op0 = 0x3;

		/* [9:4]: 000000, [5], [9] are default values */
		inst_bytes->general0.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [47:46]: 00, (default value not sure) */
		inst_bytes->general0.src2_mod = 0x0;

		/* [48]: 0, dst.cc (default value) */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: 000000000 */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 010110 */
		inst_bytes->general0.op1 = 0x16;

		break;

	case FRM_FMT_INT_SHL:

		/* [3:0]: 0011 */
		inst_bytes->general0.op0 = 0x3;

		/* [9:4]: 000000, [5], [9] are default values */
		inst_bytes->general0.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [47:46]: 00, (default value not sure) */
		inst_bytes->general0.src2_mod = 0x0;

		/* [48]: 0, dst.cc (default value) */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: 000000000 */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 011000 */
		inst_bytes->general0.op1 = 0x18;

		break;

	//case FRM_FMT_INT_LOP:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	//case FRM_FMT_INT_LOP32I:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	//case FRM_FMT_INT_FLO:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	//case FRM_FMT_INT_ISET:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	case FRM_FMT_INT_ISETP:
		
		inst_bytes->general1.op0 = 0x3;
		/* bit [4] = 0, [9:6] = 0, others default */
		inst_bytes->general1.mod0 = 0x2;
		inst_bytes->general1.dst_cc = 0x0;
		inst_bytes->general1.op1 = 0x3;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general1.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general1.pred = 0x7;
		}

		break;

//	case FRM_FMT_INT_INT_ICMP:
//		
//		inst_bytes->exp.enc = 0x3e;
//		/* No opcode: only 1 instruction */
//		break;
//
//	case FRM_FMT_CONV_F2F:
//		
//		inst_bytes->exp.enc = 0x3e;
//		/* No opcode: only 1 instruction */
//		break;
//
//	case FRM_FMT_CONV_I2F:
//		
//		inst_bytes->exp.enc = 0x3e;
//		/* No opcode: only 1 instruction */
//		break;
//
//	case FRM_FMT_CONV_I2I:
//		
//		inst_bytes->exp.enc = 0x3e;
//		/* No opcode: only 1 instruction */
//		break;

	case FRM_FMT_MOV_MOV:
		
		inst_bytes->general0.op0 = 0x4;
		inst_bytes->general0.op1 = 0xa;
		inst_bytes->general0.mod0 = 0x1e;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		break;

	case FRM_FMT_MOV_MOV32I:

		/* [3:0]: 0010 */
		inst_bytes->imm.op0 = 0x2;

		/* [9:4]: 011110 */
		inst_bytes->imm.mod0 = 0x1e;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->imm.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->imm.pred = 0x7;
		}

		/* [25:20]: 000000 */
		inst_bytes->imm.src1 = 0x0;

		/* [63:58]: 000110 */
		inst_bytes->imm.op1 = 0x0;

		break;

	//case FRM_FMT_MOV_SEL:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	case FRM_FMT_LDST_LD:
		
		/* use offset format */
		inst_bytes->offs.op0 = 0x5;
		/* [4] = 0, others default */
		inst_bytes->offs.mod0 = 0x8;
		inst_bytes->offs.mod1 = 0x0;
		/* [63:59] = 10000, [58] default */
		inst_bytes->offs.op1 = 0x20;
		/* No opcode: only 1 instruction */

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->offs.pred = inst->pred_num;
		}
		else
		{
			/*no predicate, value=7 */
			inst_bytes->offs.pred = 0x7;
		}
		break;

	//case FRM_FMT_LDST_LDS:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	case FRM_FMT_LDST_ST:
		
		inst_bytes->offs.op0 = 0x5;
		/* [4]=0, others default */
		inst_bytes->offs.mod0 = 0x8;
		inst_bytes->offs.mod1 = 0x0;
		/* [13:10], default is always predicate true, pt7 */
		inst_bytes->general0.pred = 0x7;
		/* [63:59] = 10010, [58] default */
		inst_bytes->offs.op1 = 0x24;
		/* No opcode: only 1 instruction */
		break;

	case FRM_FMT_LDST_STS:

		/* [3:0]: 0101 */
		inst_bytes->offs.op0 = 0x5;

		/* [9:4]: all 0s, default value */
		inst_bytes->offs.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->offs.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->offs.pred = 0x7;
		}

		/* [57:42]: all 0s */
		inst_bytes->offs.mod1 = 0x0;

		/* [63:58]: 110010 */
		inst_bytes->offs.op1 = 0x32;

		break;

	case FRM_FMT_CTRL_BRA:

		/* [3:0]: 0111 */
		inst_bytes->tgt.op0 = 0x7;

		/* [9:4]: 011110 */
		inst_bytes->tgt.mod0 = 0x1e;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->tgt.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->tgt.pred = 0x7;
		}

		/* [14]: tgt_mod, immediate or const mem addr, check the type 
		 * of the 1st argument */
		arg = list_get(inst->arg_list, 0);

		if (arg->type == frm_arg_literal)
			inst_bytes->tgt.tgt_mod = 0x0;
		else if (arg->type == frm_arg_const_maddr)
			inst_bytes->tgt.tgt_mod = 0x1;
		else
			frm2bin_yyerror_fmt("SSY: wrong target type!\n");

		/* [15]: 0, default value */
		inst_bytes->tgt.u = 0x0;

		/* [16]: 0, default value */
		inst_bytes->tgt.noinc = 0x0;

		/* [25:17]: all 0s */
		inst_bytes->tgt._const0 = 0x0;

		/* [57:46]: all 0s */
		inst_bytes->tgt._reserved0 = 0x0;

		/* [63:58]: 010000 */
		inst_bytes->tgt.op1 = 0x10;

		break;

	//case FRM_FMT_CTRL_CAL:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	case FRM_FMT_CTRL_RET:

		/* [3:0]: 0111 */
		inst_bytes->general0.op0 = 0x7;

		/* [9:4]: 011110 */
		inst_bytes->general0.mod0 = 0x1e;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [19:14]: all 0s */
		inst_bytes->general0.dst = 0x0;

		/* [25:20]: all 0s */
		inst_bytes->general0.src1 = 0x0;

		/* [45:26]: all 0s */
		inst_bytes->general0.src2 = 0x0;

		/* [47:46]: all 0s */
		inst_bytes->general0.src2_mod = 0x0;

		/* [48]: all 0s */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: all 0s */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 100100 */
		inst_bytes->general0.op1 = 0x24;

		break;

	case FRM_FMT_CTRL_CONT:

		/* [3:0]: 0111 */
		inst_bytes->general0.op0 = 0x7;

		/* [9:4]: 011110 */
		inst_bytes->general0.mod0 = 0x1e;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		/* [19:14]: all 0s */
		inst_bytes->general0.dst = 0x0;

		/* [25:20]: all 0s */
		inst_bytes->general0.src1 = 0x0;

		/* [45:26]: all 0s */
		inst_bytes->general0.src2 = 0x0;

		/* [47:46]: all 0s */
		inst_bytes->general0.src2_mod = 0x0;

		/* [48]: all 0s */
		inst_bytes->general0.dst_cc = 0x0;

		/* [57:49]: all 0s */
		inst_bytes->general0.mod1 = 0x0;

		/* [63:58]: 101100 */
		inst_bytes->general0.op1 = 0x2c;

		break;

	case FRM_FMT_CTRL_EXIT:

		inst_bytes->general0.op0 = 0x7;
		inst_bytes->general0.mod0 = 0x1e;
		inst_bytes->general0.dst = 0x0;
		inst_bytes->general0.src1 = 0x0;
		inst_bytes->general0.src2 = 0x0;
		inst_bytes->general0.src2_mod = 0x0;
		inst_bytes->general0.dst_cc = 0x0;
		inst_bytes->general0.mod1 = 0x0;
		inst_bytes->general0.op1 = 0x20;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}

		break;

	case FRM_FMT_CTRL_SSY:

		/* [3:0]: 0111 */
		inst_bytes->tgt.op0 = 0x7;

		/* [9:4]: 000000 */
		inst_bytes->tgt.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->tgt.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->tgt.pred = 0x7;
		}

		/* [14]: tgt_mod, immediate or const mem addr, check the type 
		 * of the 1st argument */
		arg = list_get(inst->arg_list, 0);

		if (arg->type == frm_arg_literal)
			inst_bytes->tgt.tgt_mod = 0x0;
		else if (arg->type == frm_arg_const_maddr)
			inst_bytes->tgt.tgt_mod = 0x1;
		else
			frm2bin_yyerror_fmt("SSY: wrong target type!\n");

		/* [25:15]: all 0s */
		inst_bytes->tgt.u = 0x0;
		inst_bytes->tgt.noinc = 0x0;
		inst_bytes->tgt._const0 = 0x0;

		/* [57:46]: all 0s */
		inst_bytes->tgt._reserved0 = 0x0;

		/* [63:58]: 011000 */
		inst_bytes->tgt.op1 = 0x18;

		break;

	case FRM_FMT_CTRL_PCNT:

		/* [3:0]: 0111 */
		inst_bytes->tgt.op0 = 0x7;

		/* [9:4]: 000000 */
		inst_bytes->tgt.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->tgt.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->tgt.pred = 0x7;
		}

		/* [14]: tgt_mod, immediate or const mem addr, check the type 
		 * of the 1st argument */
		arg = list_get(inst->arg_list, 0);

		if (arg->type == frm_arg_literal)
			inst_bytes->tgt.tgt_mod = 0x0;
		else if (arg->type == frm_arg_const_maddr)
			inst_bytes->tgt.tgt_mod = 0x1;
		else
			frm2bin_yyerror_fmt("SSY: wrong target type!\n");

		/* [15]: 0, default value */
		inst_bytes->tgt.u = 0x0;

		/* [16]: 0, default value */
		inst_bytes->tgt.noinc = 0x0;

		/* [25:17]: all 0s */
		inst_bytes->tgt._const0 = 0x0;

		/* [57:46]: all 0s */
		inst_bytes->tgt._reserved0 = 0x0;

		/* [63:58]: 011100 */
		inst_bytes->tgt.op1 = 0x1c;

		break;

	case FRM_FMT_MISC_NOP:

		/* [3:0]: 0100 */
		inst_bytes->offs.op0 = 0x4;

		/* [9:4]: all 0s, default value */
		inst_bytes->offs.mod0 = 0x0;

		/* [13:10]: pred */
		if (inst->pred_num >= 0)
		{
			inst_bytes->offs.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->offs.pred = 0x7;
		}

		/* [19:14]: all 0s */
		inst_bytes->offs.dst = 0x0;

		/* [25:20]: all 0s */
		inst_bytes->offs.src1 = 0x0;

		/* [41:26]: all 0s, default value */
		inst_bytes->offs.offset = 0x0;

		/* [57:42]: default value */
		inst_bytes->offs.mod1 = 0x0;

		/* [63:58]: 010000 */
		inst_bytes->offs.op1 = 0x10;

		break;

	case FRM_FMT_MISC_S2R:

		inst_bytes->general0.op0 = 0x4;
		inst_bytes->general0.mod0 = 0x0;
		inst_bytes->general0.src1 = 0x0;
		inst_bytes->general0.src2_mod = 0x0;
		inst_bytes->general0.dst_cc = 0x0;
		inst_bytes->general0.mod1 = 0x0;
		inst_bytes->general0.op1 = 0xb;

		/* set the predicate */
		if (inst->pred_num >= 0)
		{
			inst_bytes->general0.pred = inst->pred_num;
		}
		else
		{
			/* no predicate, value=7 */
			inst_bytes->general0.pred = 0x7;
		}
		/* No opcode: only 1 instruction */
		break;

	//case FRM_FMT_MISC_BAR:
	//	
	//	inst_bytes->exp.enc = 0x3e;
	//	/* No opcode: only 1 instruction */
	//	break;

	default:
		fatal("%s: unsupported format", __FUNCTION__);
	}

	/* process modifier list */
	/* this part should be improved later */
	if (inst->mod_list->count > 0)
	{
		LIST_FOR_EACH(inst->mod_list, index)
		{
			int data_width_cnt = 0;

			mod = list_get(inst->mod_list, index);
			/* take actions according to the type of the modifier 
			 */
			switch (mod->type)
			{
			case frm_token_mod_data_width:
			{
				/* right now I only care about IMADD inst, *
				 * this should be improved later */
				if (data_width_cnt == 0)
				{
					data_width_cnt++;
					/* for wide1 */
					if (mod->value.data_width == u32)
					{
						inst_bytes->general0.mod0
							&= 0xfffd;
					}
					else
					{
						inst_bytes->general0.mod0
							|= 0x2;
					}
				}
				else if (data_width_cnt == 1)
				{
					/* reset the counter */
					data_width_cnt = 0;
					/* for wide2 */
					if (mod->value.data_width == u32)
					{
						inst_bytes->general0.mod0
							&= 0xfff7;
					}
					else
					{
						inst_bytes->general0.mod0
							|= 0x8;
					}
				}
				else
				{
					frm2bin_yyerror_fmt
						("Error unrecognized \
							mod data_width \n");
				}
				break;
			}

			case frm_token_mod_logic:
			{
				/* right now only cares about isetp inst,
				 * need improvement later */
				if (mod->value.logic == logic_and)
				{
					inst_bytes->general1.logic = 0x0;
				}
				else if (mod->value.logic == logic_or)
				{
					inst_bytes->general1.logic = 0x1;
				}
				else if (mod->value.logic == logic_xor)
				{
					inst_bytes->general1.logic = 0x2;
				}
				else
				{
					frm2bin_yyerror_fmt
						("Error unrecognized \
							mod logic \n");
				}

				break;
			}

			case frm_token_mod_comparison:
			{
				/* right now only cares about isetp inst,
				 * need improvement later */
				if (mod->value.comparison == lt)
				{
					inst_bytes->general1.cmp = 0x1;
				}
				else if (mod->value.comparison == eq)
				{
					inst_bytes->general1.cmp = 0x2;
				}
				else if (mod->value.comparison == le)
				{
					inst_bytes->general1.cmp = 0x3;
				}
				else if (mod->value.comparison == gt)
				{
					inst_bytes->general1.cmp = 0x4;
				}
				else if (mod->value.comparison == ne)
				{
					inst_bytes->general1.cmp = 0x5;
				}
				else if (mod->value.comparison == ge)
				{
					inst_bytes->general1.cmp = 0x6;
				}
				else
				{
					frm2bin_yyerror_fmt
						("Error unrecognized \
							mod comparison \n");
				}

				break;
			}

			case frm_token_mod0_B_brev:
			{
				/* bit reverse */
				if (mod->value.brev == 1)
					inst_bytes->mod0_B.cop |= 0x1;
				else
					inst_bytes->mod0_B.cop &= 0xfe;

				break;
			}

			case frm_token_mod0_B_type:
			{
				/* data type */
				if (mod->value.mod0_B_type == 0)
					inst_bytes->mod0_B.type = 0x0;
				else if (mod->value.mod0_B_type == 1)
					inst_bytes->mod0_B.type = 0x1;
				else if (mod->value.mod0_B_type == 2)
					inst_bytes->mod0_B.type = 0x2;
				else if (mod->value.mod0_B_type == 3)
					inst_bytes->mod0_B.type = 0x3;
				else if (mod->value.mod0_B_type == 4)
					inst_bytes->mod0_B.type = 0x4;
				else if (mod->value.mod0_B_type == 5)
					inst_bytes->mod0_B.type = 0x5;
				else if (mod->value.mod0_B_type == 6)
					inst_bytes->mod0_B.type = 0x6;
				else
				{
					frm2bin_yyerror_fmt
						("Error unrecognized \
							mod0_B_type \n");
				}

				break;
			}

			case frm_token_tgt_u:
			{
				/* target u? */
				if (mod->value.tgt_u == 1)
					inst_bytes->tgt.u = 0x1;
				else
					inst_bytes->tgt.u = 0x0;

				break;
			}

			case frm_token_tgt_lmt:
			{
				/* target lmt? */
				if (mod->value.tgt_lmt == 1)
					inst_bytes->tgt.noinc = 0x1;
				else
					inst_bytes->tgt.noinc = 0x0;

				break;
			}

			case frm_token_mod0_A_w:
			{
				/* width? only mod0_A.neg_src1 matches,
				 * change later? */
				if (mod->value.mod0_A_w == 1)
					inst_bytes->mod0_A.neg_src1 = 0x1;
				else
					inst_bytes->mod0_A.neg_src1 = 0x0;

				break;
			}

			case frm_token_mod0_D_ftzfmz:
			{
				if (mod->value.mod0_D_ftzfmz == 1)
					inst_bytes->mod0_D.ftzfmz = 0x1;
				else if (mod->value.mod0_D_ftzfmz == 2)
					inst_bytes->mod0_D.ftzfmz = 0x2;
				else
					/* invalid mod0_D_ftzfmz */
					inst_bytes->mod0_D.ftzfmz = 0x3;

				break;
			}

			case frm_token_mod0_D_sat:
			{
				if (mod->value.mod0_D_sat == 1)
					inst_bytes->mod0_D.sat = 0x1;
				else
					inst_bytes->mod0_D.sat = 0x0;

				break;
			}

			case frm_token_gen0_mod1_B_rnd:
			{
				if (mod->value.gen0_mod1_B_rnd == 0)
					inst_bytes->general0_mod1_B.rnd = 0x0;
				else if (mod->value.gen0_mod1_B_rnd == 1)
					inst_bytes->general0_mod1_B.rnd = 0x1;
				else if (mod->value.gen0_mod1_B_rnd == 2)
					inst_bytes->general0_mod1_B.rnd = 0x2;
				else
					inst_bytes->general0_mod1_B.rnd = 0x3;

				break;
			}

			case frm_token_offs_mod1_A_trig:
			{
				if (mod->value.offs_mod1_A_trig == 1)
					inst_bytes->offs_mod1_A.trig = 0x1;
				else
					inst_bytes->offs_mod1_A.trig = 0x0;

				break;
			}

			case frm_token_offs_mod1_A_op:
			{
				if (mod->value.offs_mod1_A_op == 1)
					inst_bytes->offs_mod1_A.op = 0x1;
				else if (mod->value.offs_mod1_A_op == 2)
					inst_bytes->offs_mod1_A.op = 0x2;
				else if (mod->value.offs_mod1_A_op == 3)
					inst_bytes->offs_mod1_A.op = 0x3;
				else if (mod->value.offs_mod1_A_op == 4)
					inst_bytes->offs_mod1_A.op = 0x4;
				else if (mod->value.offs_mod1_A_op == 5)
					inst_bytes->offs_mod1_A.op = 0x5;
				else if (mod->value.offs_mod1_A_op == 6)
					inst_bytes->offs_mod1_A.op = 0x6;
				else if (mod->value.offs_mod1_A_op == 7)
					inst_bytes->offs_mod1_A.op = 0x7;
				else if (mod->value.offs_mod1_A_op == 8)
					inst_bytes->offs_mod1_A.op = 0x8;
				else
					inst_bytes->offs_mod1_A.op = 0x0;

				break;
			}

			default:
				frm2bin_yyerror_fmt("Error which figure out \
						the modifier \n");

			}
		}
	}


	/* Arguments */
	assert(inst->arg_list->count == info->token_list->count);
	LIST_FOR_EACH(inst->arg_list, index)
	{
		/* Get argument */
		arg = list_get(inst->arg_list, index);
		token = list_get(info->token_list, index);
		assert(arg);
		assert(token);

		/* Check token */
		switch (token->type)
		{
			/* only support the argument I need now */
		case frm_token_dst:
		{
			/* [19:14]dst, both general0 and general1 use it */
			if (arg->type == frm_arg_scalar_register)
			{
				inst_bytes->general0.dst =
					arg->value.scalar_register.id;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_dst. \
					[dis-inst.c]\n");
			}

			break;
		}

		case frm_token_src1:
		{
			/* [25:20] src2 */
			if (arg->type == frm_arg_scalar_register)
			{
				/* for scalar register */
				inst_bytes->general0.src1 =
					arg->value.scalar_register.id;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_src1. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_src2:
		{
			/* [45:26] src2 */
			if (arg->type == frm_arg_const_maddr)
			{
				/* for const mem */
				inst_bytes->general0.src2_mod = 0x1;
				inst_bytes->general0.src2 =
					(arg->value.glob_maddr.reg_idx
					<< 16)
					| (arg->value.glob_maddr.offset);
			}

			else if (arg->type == frm_arg_scalar_register)
			{
				/* for scalar register */
				inst_bytes->general0.src2_mod = 0x0;
				inst_bytes->general0.src2 =
					arg->value.scalar_register.id;
			}

			else if (arg->type == frm_arg_zero_register)
			{
				/* [45:26]: src2, all 0s Register Zero, reg
				 * that contains const value of 0 */
				inst_bytes->general0.src2 = 0xfffff;
			}

			else if (arg->type == frm_arg_literal)
			{
				/* for immediate value */
				inst_bytes->general0.src2_mod = 0x2;
				inst_bytes->general0.src2 =
					arg->value.literal.val;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_src2. \
					[inst.c]\n");
			}
			break;
		}

		case frm_token_src2_neg:
		{
		        /* [45:26] src2 */
		        //if (arg->type == frm_arg_const_maddr)
		        //{
		        //        /* for const mem */
		        //        inst_bytes->general0.src2_mod = 0x1;
		        //        inst_bytes->general0.src2 = 
		        //        	(arg->value.glob_maddr.reg_idx << 16)
		        //      	| (arg->value.glob_maddr.offset);
		        //}

		        if (arg->type == frm_arg_scalar_register)
		        {
		      		/* need to be imporved later, src2_neg 
		      	  	 * need way to identify negative */
		      	  	inst_bytes->general0.src2 = 
		      	  		arg->value.scalar_register.id;
		        }

		        //else if (arg->type == frm_arg_literal)
		        //{
		        //        /* for immediate value */
		        //        inst_bytes->general0.src2_mod = 0x2;
		        //        inst_bytes->general0.src2 = 
		        //        	arg->value.literal.val;
		        //}
		        else
		        {
		  		frm2bin_yyerror_fmt("Wrong frm_token_src2_neg. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_src3:
		{
			if (arg->type == frm_arg_scalar_register)
			{
				/* [54:49] */
				inst_bytes->general0_mod1_B.src3 =
					arg->value.scalar_register.id;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_src3. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_src2_frm_sr:
		{
			switch (arg->value.special_register.type)
			{
				/* frm_fmt_imm_t is very similar */
			case FRM_SR_CTAid_X:
			{
				inst_bytes->imm.imm32 = 37;
				break;
			}
			case FRM_SR_Tid_X:
			{
				inst_bytes->imm.imm32 = 33;
				break;
			}
			default:
			{
				frm2bin_yyerror_fmt("Not supported \
						special register\n");
				break;
			}
				/* other registers added later */
			}
			break;
		}

		case frm_token_tgt:
		{
			if (arg->type == frm_arg_literal)
			{
				inst_bytes->tgt.target =
					arg->value.literal.val;
			}
			else
				frm2bin_yyerror_fmt
					("Illegal frm_token_tgt type!\n");
			break;
		}

		case frm_token_offs:
		{
			/* []: */
			break;
		}

		case frm_token_imm32:
		{
			/* [57:26]: imm32 */
			if (arg->type == frm_arg_literal)
			{
				inst_bytes->imm.imm32 =
					arg->value.literal.val;
			}
			break;
		}

		case frm_token_mod0_C_shamt:
		{
			if (arg->type == frm_arg_literal)
			{
				/* [9:5] */
				inst_bytes->mod0_C.shamt =
					arg->value.literal.val;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_shamt. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_Q:
		{
			if (arg->type == frm_arg_pt)
			{
				/* [16:14], general0.dst[19:14] */
				inst_bytes->general0.dst =
					arg->value.pt.idx
					| (inst_bytes->general0.dst & 0x38);
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_Q. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_P:
		{
			/* a new frm_fmt need to be created in asm.h to
			 * handle the case of P */
			if (arg->type == frm_arg_predicate_register)
			{
				/* [19:17], general0.dst is [19:14] */
				inst_bytes->general0.dst =
					(arg->value.predicate_register.id
					<< 3)
					| (inst_bytes->general0.dst & 0x7);
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_P. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_R:
		{
			if (arg->type == frm_arg_pt)
			{
				/* [51:49] */
				/* will be improved later */
				inst_bytes->general1.R = arg->value.pt.idx;
			}
			else
			{
				frm2bin_yyerror_fmt("Wrong frm_token_R. \
					[dis-inst.c]\n");
			}
			break;
		}

			/* temporial for LD dst, [src1 + offset] src1 is
			 * combined with offset now */
		case frm_token_src1_offs:
		{
			if (arg->type == frm_arg_glob_maddr)
			{
				/* [25:20] */
				inst_bytes->general0.src1 =
					arg->value.glob_maddr.reg_idx;
			}
			else
			{
				frm2bin_yyerror_fmt
					("Wrong frm_token_src1_offs. \
					[dis-inst.c]\n");
			}
			break;
		}

		case frm_token_mod0_C_ccop:
		{
			if (arg->type == frm_arg_ccop)
				/* maybe name "shamt" shoule be changed
				 * later? */
				inst_bytes->mod0_C.shamt = arg->value.ccop.op;

			break;
		}

		default:
			frm2bin_yyerror_fmt
				("unsupported token for argument %d",
				index + 1);
		}
	}
}

/* functions for predicate structure creation and free */
struct frm2bin_pred_t *frm2bin_pred_create(int number)
{
	struct frm2bin_pred_t *pred;

	pred = xcalloc(1, sizeof(struct frm2bin_pred_t));
	pred->number = number;

	return pred;
}

void frm2bin_pred_free(struct frm2bin_pred_t *pred)
{
	free(pred);
}