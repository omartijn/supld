include(${CMAKE_SOURCE_DIR}/asn1c-cmake-module/ASN1.cmake)

asn1_add_module_library(
    supl 
        rrlp-components.asn
        supl-common.asn
        supl-init.asn
        supl-posinit.asn
        supl-start.asn
        rrlp-messages.asn
        supl-end.asn
        supl-pos.asn
        supl-response.asn
        supl-ulp.asn
    ASN1C_OPTIONS
        -gen-PER
)
