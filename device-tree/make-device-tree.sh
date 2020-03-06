common_args=()
common_args+=("--input ../quartus-project/soc_system.sopcinfo")
common_args+=("--board custom_info.xml")
common_args+=("--board soc_system_board_info.xml")
common_args+=("--board hps_common_board_info.xml")
common_args+=("--bridge-removal all")
common_args+=("--clocks -v")

dts_args=("${common_args[@]}")
dts_args+=("--type dts")
dts_args+=("--output socfpga.dts")
dts_args=$(IFS=" " ; echo "${dts_args[*]}")
java -jar sopc2dts.jar $dts_args   

dtb_args=("${common_args[@]}")
dtb_args+=("--type dtb")
dtb_args+=("--output socfpga.dtb")
dtb_args=$(IFS=" " ; echo "${dtb_args[*]}")
java -jar sopc2dts.jar $dtb_args   