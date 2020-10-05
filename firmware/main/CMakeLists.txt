idf_component_register(SRCS "main.c" "wifi.c" "bme280.c" "http.c" "led.c" "ulp.c" INCLUDE_DIRS "")

set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_s_sources ulp/main.S)
set(ulp_exp_dep_srcs "ulp.c")

ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")
