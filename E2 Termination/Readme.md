# Running in Terminal

    export RMR_SEED_RT=rtable.rt RMR_LOG_VLEVEL=0
	gcc E2_Termination.c -g -o E2T_name -L. -lasncodec -lsctp -lrmr_si -lpthread cJSON.c -lm
	./E2T_name