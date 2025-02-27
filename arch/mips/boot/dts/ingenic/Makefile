dtb-$(CONFIG_JZ4740_QI_LB60)	+= qi_lb60.dtb
dtb-$(CONFIG_JZ4780_CI20)	+= ci20.dtb
dtb-$(CONFIG_DT_HALLEY2_V20)	+= halley2v20.dtb
dtb-$(CONFIG_DT_SEAL)		+= seal.dtb
dtb-$(CONFIG_DT_X2000_V12_FPGA)	+= x2000_v12_fpga.dtb
dtb-$(CONFIG_DT_T40_SHARK)	+= shark.dtb
dtb-$(CONFIG_DT_A1_TUCANA)	+= tucana.dtb
dtb-$(CONFIG_DT_A1_GORILLA)	+= gorilla.dtb
dtb-$(CONFIG_DT_A1_CALF)	+= a1_calf.dtb
dtb-$(CONFIG_DT_A1_CALF_V12)	+= a1_calf_v12.dtb
dtb-$(CONFIG_DT_A1_CALF_V13)	+= a1_calf_v13.dtb
dtb-$(CONFIG_DT_A1_CALF_V16)	+= a1_calf_v16.dtb


obj-y				+= $(patsubst %.dtb, %.dtb.o, $(dtb-y))

# Force kbuild to make empty built-in.o if necessary
obj-				+= dummy.o

always				:= $(dtb-y)
clean-files			:= *.dtb *.dtb.S
