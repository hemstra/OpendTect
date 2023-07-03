#________________________________________________________________________
#
# Copyright:    (C) 1995-2022 dGB Beheer B.V.
# License:      https://dgbes.com/licensing
#________________________________________________________________________
#

macro( OD_SETUP_OPENSSL_TARGET TRGT )

    get_target_property( OPENSSL_CONFIGS ${TRGT} IMPORTED_CONFIGURATIONS )
    if ( OPENSSL_CONFIGS )
	foreach( config ${OPENSSL_CONFIGS} )
	    get_target_property( OPENSSL_LOCATION_${config} ${TRGT} IMPORTED_LOCATION_${config} )
	    if ( OPENSSL_LOCATION_${config} )
		if ( WIN32 )
		    set( OPENSSL_IMPLIB_LOCATION_${config} "${OPENSSL_LOCATION_${config}}" )
		    od_get_dll( "${OPENSSL_IMPLIB_LOCATION_${config}}" OPENSSL_LOCATION_${config} )
		else()
		    get_filename_component( OPENSSL_LOCATION_${config} "${OPENSSL_LOCATION_${config}}" REALPATH )
		endif()
		set_target_properties( ${TRGT} PROPERTIES
		    IMPORTED_LOCATION_${config} "${OPENSSL_LOCATION_${config}}" )
		unset( OPENSSL_IMPORTED_LOCATION_${config} )
		if ( WIN32 )
		    set_target_properties( ${TRGT} PROPERTIES
			IMPORTED_IMPLIB_${config} "${OPENSSL_IMPLIB_LOCATION_${config}}" )
		    unset( OPENSSL_IMPLIB_LOCATION_${config} )
		endif()
	    endif()
	endforeach()
	unset( OPENSSL_CONFIGS )
    else()
	get_target_property( OPENSSL_LOCATION ${TRGT} IMPORTED_LOCATION )
	if ( OPENSSL_LOCATION )
	    if ( WIN32 )
		set( OPENSSL_IMPLIB_LOCATION "${OPENSSL_LOCATION}" )
		od_get_dll( "${OPENSSL_IMPLIB_LOCATION}" OPENSSL_LOCATION )
	    else()
		get_filename_component( OPENSSL_LOCATION "${OPENSSL_LOCATION}" REALPATH )
	    endif()
	    set_target_properties( ${TRGT} PROPERTIES
		    IMPORTED_CONFIGURATIONS RELEASE
		    IMPORTED_LOCATION_RELEASE "${OPENSSL_LOCATION}" )
	    if ( WIN32 )
		set_target_properties( ${TRGT} PROPERTIES
				       IMPORTED_IMPLIB_RELEASE "${OPENSSL_IMPLIB_LOCATION}" )
		unset( OPENSSL_IMPLIB_LOCATION )
	    endif()
	else()
	    message( SEND_ERROR "Cannot retrieve OpenSSL import location" )
	endif()
    endif()
    od_map_configurations( ${TRGT} )

endmacro(OD_SETUP_OPENSSL_TARGET)

macro( OD_ADD_QTOPENSSL_HINT )
    if ( EXISTS "${QT_DIR}" )
	get_filename_component( QTINSTDIR ${QT_DIR} REALPATH )
	set( OPENSSL_HINT_DIR "${QTINSTDIR}/../../../../../Tools/OpenSSL" )
	get_filename_component( OPENSSL_HINT_DIR ${OPENSSL_HINT_DIR} REALPATH )
	if ( EXISTS ${OPENSSL_HINT_DIR} )
	    set( OPENSSL_ROOT_DIR ${OPENSSL_HINT_DIR} )
	    if ( WIN32 )
		set( OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}/Win_x64 )
	    elseif( NOT DEFINED APPLE )
		set( OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR}/binary )
	    endif( WIN32 )
	endif()
    endif( EXISTS "${QT_DIR}" )
endmacro( OD_ADD_QTOPENSSL_HINT )

macro( OD_CLEANUP_OPENSSL )
    unset( OPENSSL_INCLUDE_DIR CACHE )
    unset( OPENSSL_CRYPTO_LIBRARY CACHE )
    unset( OPENSSL_SSL_LIBRARY CACHE )
    unset( OPENSSL_SSL_VERSION CACHE )
    unset( LIB_EAY_DEBUG CACHE )
    unset( LIB_EAY_LIBRARY_DEBUG CACHE )
    unset( LIB_EAY_LIBRARY_RELEASE CACHE )
    unset( LIB_EAY_RELEASE CACHE )
    unset( SSL_EAY_DEBUG CACHE )
    unset( SSL_EAY_LIBRARY_DEBUG CACHE )
    unset( SSL_EAY_LIBRARY_RELEASE CACHE )
    unset( SSL_EAY_RELEASE CACHE )
endmacro(OD_CLEANUP_OPENSSL)

macro( OD_SETUP_OPENSSLCOMP COMP )
    list( APPEND OD_MODULE_COMPILE_DEFINITIONS
	  "__OpenSSL_${COMP}_LIBRARY__=\"$<TARGET_FILE:OpenSSL::${COMP}>\"" )
endmacro(OD_SETUP_OPENSSLCOMP)

macro( OD_FIND_OPENSSL )

    if ( NOT TARGET OpenSSL::SSL OR NOT TARGET OpenSSL::Crypto )
	find_package( OpenSSL 1.1.1 QUIET COMPONENTS SSL Crypto GLOBAL )
	if ( NOT TARGET OpenSSL::SSL OR NOT TARGET OpenSSL::Crypto )
	    OD_CLEANUP_OPENSSL()
	    unset( OPENSSL_FOUND )
	    OD_ADD_QTOPENSSL_HINT()
	    find_package( OpenSSL 1.1.1 QUIET COMPONENTS SSL Crypto GLOBAL )
	endif()
	if ( OPENSSL_FOUND )
	    OD_SETUP_OPENSSL_TARGET( OpenSSL::SSL )
	    OD_SETUP_OPENSSL_TARGET( OpenSSL::Crypto )
	else()
	    OD_CLEANUP_OPENSSL()
	endif()
    endif()

endmacro(OD_FIND_OPENSSL)

macro( OD_SETUP_CRYPTO )

    if ( OPENSSL_FOUND AND TARGET OpenSSL::Crypto )
	if ( OD_LINKCRYPTO )
	    list( APPEND OD_MODULE_EXTERNAL_LIBS OpenSSL::Crypto )
	elseif ( OD_USECRYPTO )
	    list( APPEND OD_MODULE_EXTERNAL_RUNTIME_LIBS OpenSSL::Crypto )
	    OD_SETUP_OPENSSLCOMP( Crypto )
	endif()
    else()
	set( OPENSSL_ROOT_DIR "" CACHE PATH "OpenSSL Location" )
	message( SEND_ERROR "Cannot find/use the OpenSSL installation" )
    endif()

endmacro(OD_SETUP_CRYPTO)

macro( OD_SETUP_OPENSSL )

    if ( OPENSSL_FOUND AND TARGET OpenSSL::SSL )
	if ( OD_LINKOPENSSL )
	    list( APPEND OD_MODULE_EXTERNAL_LIBS OpenSSL::SSL )
	elseif( OD_USEOPENSSL )
	    list( APPEND OD_MODULE_EXTERNAL_RUNTIME_LIBS OpenSSL::SSL )
	    OD_SETUP_OPENSSLCOMP( SSL )
	endif()
    else()
	set( OPENSSL_ROOT_DIR "" CACHE PATH "OpenSSL Location" )
	message( SEND_ERROR "Cannot find/use the OpenSSL installation" )
    endif()

endmacro(OD_SETUP_OPENSSL)

