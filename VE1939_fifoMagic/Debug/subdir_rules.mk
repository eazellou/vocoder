################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Copy\ of\ hailMary.obj: ../Copy\ of\ hailMary.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C5500 Compiler'
	"C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/include" --include_path="N:/windat.V2/workspace_v6_1/C5515_Support_Files/452_Support" --display_error_number --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="Copy of hailMary.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hailMary.obj: ../hailMary.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C5500 Compiler'
	"C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/VApps/ticcs/6p1p0/ccsv6/tools/compiler/c5500_4.4.1/include" --include_path="N:/windat.V2/workspace_v6_1/C5515_Support_Files/452_Support" --display_error_number --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="hailMary.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


