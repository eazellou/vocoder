################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
eqtest.obj: ../eqtest.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C5500 Compiler'
	"C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/include" --include_path="N:/windat.V2/workspace_v6_1/452_Support" --include_path="N:/windat.V2/workspace_v6_1/C5515_Support_Files/452_Support" --display_error_number --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="eqtest.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

iircasNew.obj: ../iircasNew.asm $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C5500 Compiler'
	"C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/include" --include_path="N:/windat.V2/workspace_v6_1/452_Support" --include_path="N:/windat.V2/workspace_v6_1/C5515_Support_Files/452_Support" --display_error_number --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="iircasNew.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


