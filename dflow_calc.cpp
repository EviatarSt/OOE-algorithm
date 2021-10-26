/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct gDP graphDP; //Graph dependency typedef struct
typedef struct nCMD nodeCMD; //command node typedef struct

#define ENTRY_ID -1 //Default entry id definition

// struct for command node
struct nCMD{
	int id;
	InstInfo* info;				// command information (opcode, dstIdx, src1Idx, src2Idx) (instInfo used from dflow_calc.h)
    nodeCMD* dp1;				// a pointer to src1 dependency
	nodeCMD* dp2;				// a pointer to src2 dependency
    unsigned int duration;		// duration of the command measured in clock cycles
	unsigned int depth;         //command depth
};

// struct for graph dependency
struct gDP {
	nodeCMD entry;				// graph entry node
	nodeCMD* pArrCMD;		    // command pointer array for graph data structure
	unsigned int numInsts;      // number of instance in the graph dependancy
	unsigned int depth;         // graph dependancy depth
};

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numInsts) {
    //Validate the input, return PROG_CTX_NULL if not true

    if (!opsLatency || !progTrace || numInsts == 0) {
		return PROG_CTX_NULL;
	}   

	//this allocates memory for the dependency graph 'graph'
	graphDP* theGraph = (graphDP*)malloc(sizeof(graphDP));
	if (!theGraph) {
		return PROG_CTX_NULL;	// in the case that the malloc fails
	}

    //this allocates memory for the commands pointer array 'pArrCMD'
	theGraph->pArrCMD = (nodeCMD*)malloc(numInsts*sizeof(nodeCMD));

	if (!(theGraph->pArrCMD)) {
		free(theGraph); //deallocate memory for graph
		return PROG_CTX_NULL;
	}

    theGraph->depth = 0;
	theGraph->numInsts = numInsts;

    
    // creating entry node
	theGraph->entry.depth = 0;
	theGraph->entry.duration = 0;
	theGraph->entry.id = ENTRY_ID;
	theGraph->entry.info = NULL;
    theGraph->entry.dp1 = NULL;
	theGraph->entry.dp2 = NULL;

	for (unsigned int i = 0 ; i < numInsts ; i++) {
		
		theGraph->pArrCMD[i].id = i;

		// allocating sufficient memory for command's info
		theGraph->pArrCMD[i].info = (InstInfo*)malloc(sizeof(InstInfo));
        
		if (!(theGraph->pArrCMD[i].info)) {
			for (unsigned int j=0 ; j<i-1 ; j++ ) {
				
				free(theGraph->pArrCMD[j].info);
				
			}
			free(theGraph->pArrCMD);
			free(theGraph);
			return PROG_CTX_NULL;
		}

        // copying command info
        theGraph->pArrCMD[i].info->opcode = progTrace[i].opcode;
		theGraph->pArrCMD[i].info->dstIdx = progTrace[i].dstIdx;
		theGraph->pArrCMD[i].info->src1Idx = progTrace[i].src1Idx;
		theGraph->pArrCMD[i].info->src2Idx = progTrace[i].src2Idx;

		// extracting command's duration
		theGraph->pArrCMD[i].duration = opsLatency[progTrace[i].opcode];

        //Set depth is set to 0, dp1 and dp2 is set to NULL
        theGraph->pArrCMD[i].depth = 0;
        theGraph->pArrCMD[i].dp1 = NULL;
		theGraph->pArrCMD[i].dp2 = NULL;

        // only past commands determines dependencies
		int k = (int) i;

		for (int j = k-1 ; j >= 0; j--) {
			// checking the dependancy of src1
			unsigned int temp = (unsigned int)theGraph->pArrCMD[j].info->dstIdx; //casts int to unsigned int from the InstInfo Struct

			if(theGraph->pArrCMD[i].info->src1Idx == temp) {

				if (theGraph->pArrCMD[i].dp1 == NULL) {

					theGraph->pArrCMD[i].dp1 = &(theGraph->pArrCMD[j]);

					// changing command's duration if the depth[i] is less then the depth[j] + the duration[j]
					if ((theGraph->pArrCMD[i].depth) < (theGraph->pArrCMD[j].depth + theGraph->pArrCMD[j].duration)) {

						theGraph->pArrCMD[i].depth = theGraph->pArrCMD[j].depth + theGraph->pArrCMD[j].duration;

					}

				}

			}
			// checking the dependancy of src2

			if (theGraph->pArrCMD[i].info->src2Idx == temp) {

				if (theGraph->pArrCMD[i].dp2 == NULL) {

				theGraph->pArrCMD[i].dp2 = &(theGraph->pArrCMD[j]);

                    // changing command's duration if the depth[i] is less then the depth[j] + the duration[j]
					if ((theGraph->pArrCMD[i].depth) < (theGraph->pArrCMD[j].depth + theGraph->pArrCMD[j].duration)) {

						theGraph->pArrCMD[i].depth = theGraph->pArrCMD[j].depth + theGraph->pArrCMD[j].duration;
	
					}

				}

			}

		}


		// changing graph's depth in case we discovered a longer path
		if (theGraph->depth < (theGraph->pArrCMD[i].depth + theGraph->pArrCMD[i].duration)) {
			theGraph->depth = theGraph->pArrCMD[i].depth + theGraph->pArrCMD[i].duration;
		}

        // in case command has no dependencies at all
		if (theGraph->pArrCMD[i].dp1 == NULL && theGraph->pArrCMD[i].dp2 == NULL) {
			theGraph->pArrCMD[i].dp1 = &(theGraph->entry);
		}

    }
    return theGraph;
}

void freeProgCtx(ProgCtx ctx) {

    	if (ctx != NULL) {

		for (unsigned int i=0 ; i<((graphDP*)ctx)->numInsts ; i++ ) {
			free(((graphDP*)ctx)->pArrCMD[i].info); // freeing allocated memory for command's info
		}

		free(((graphDP*)ctx)->pArrCMD);
		free(ctx);
	}
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {

    // validating input
	if (ctx == NULL || theInst >= ((graphDP*)ctx)->numInsts) {
		return -1;
	}

    return (((graphDP*)ctx)->pArrCMD[theInst].depth);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    // validating input
	if (ctx == NULL || theInst >= ((graphDP*)ctx)->numInsts){
		return -1;
    }

	// dp1 and dp2 cannot both point to null
	if (((graphDP*)ctx)->pArrCMD[theInst].dp1 == NULL) {
		*src1DepInst = ENTRY_ID;
		*src2DepInst = ((graphDP*)ctx)->pArrCMD[theInst].dp2->id;
		return 0;
	}
    
	// in case of no dependencies
	else if (((graphDP*)ctx)->pArrCMD[theInst].dp1->id == ENTRY_ID) {
		*src1DepInst = ENTRY_ID;
		*src2DepInst = ENTRY_ID;
		return 0;
	}
	else {
		*src1DepInst = ((graphDP*)ctx)->pArrCMD[theInst].dp1->id;
        
		if (((graphDP*)ctx)->pArrCMD[theInst].dp2 == NULL)
			*src2DepInst = ENTRY_ID;
		else
			*src2DepInst = ((graphDP*)ctx)->pArrCMD[theInst].dp2->id;
		return 0;
	}
    return -1;
}

int getProgDepth(ProgCtx ctx) {
	if (ctx == NULL)
		return 0;
	else
		return ((graphDP*)ctx)->depth;
}

