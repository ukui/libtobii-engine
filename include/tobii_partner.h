/*
COPYRIGHT 2015 - PROPERTY OF TOBII AB
-------------------------------------
2015 TOBII AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

NOTICE:  All information contained herein is, and remains, the property of Tobii AB and its suppliers, if any.
The intellectual and technical concepts contained herein are proprietary to Tobii AB and its suppliers and may be
covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law.
Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
permission is obtained from Tobii AB.
*/

#ifndef tobii_partner_h_included
#define tobii_partner_h_included

#include "tobii.h"

#ifdef __cplusplus

extern "C" {
#endif

typedef enum tobii_presence_status_t
{
    TOBII_PRESENCE_STATUS_UNKNOWN,
    TOBII_PRESENCE_STATUS_AWAY,
    TOBII_PRESENCE_STATUS_PRESENT,
} tobii_presence_status_t;

typedef struct tobii_presence_t
{
    int64_t timestamp_us;
    tobii_presence_status_t status;
} tobii_presence_t;

typedef void( *tobii_presence_callback_t )( tobii_presence_t const* presence, void* user_data );
TOBII_API tobii_error_t TOBII_CALL tobii_presence_subscribe(
    tobii_device_t* device, tobii_presence_callback_t callback, void* user_data );
TOBII_API tobii_error_t TOBII_CALL tobii_presence_unsubscribe( tobii_device_t* device );


typedef struct tobii_low_frequency_head_position_t
{
    int64_t timestamp_us;
    tobii_validity_t validity;
    float position_xyz[ 3 ];
} tobii_low_frequency_head_position_t;

typedef void (*tobii_head_position_callback_t)( tobii_low_frequency_head_position_t const* position, void* user_data );
TOBII_API tobii_error_t TOBII_CALL tobii_low_frequency_head_position_subscribe( tobii_device_t* device,
    tobii_head_position_callback_t callback, void* user_data );
TOBII_API tobii_error_t TOBII_CALL tobii_low_frequency_head_position_unsubscribe( tobii_device_t* device );

#ifdef __cplusplus
}
#endif

#endif /* tobii_partner_h_included */

/**
@defgroup tobii_partner tobii_partner.h

tobii_partner.h
=======

The tobii_partner.h file contains features exclusive to Tobii partners under professional license.

Please note that there can only be one callback registered to a stream at a time. To register a new
callback, first unsubscribe from the stream, then resubscribe with the new callback function.

Do NOT call StreamEngine API functions from within the callback functions, due to risk of internal
deadlocks. Generally one should finish the callback functions as quickly as possible and not make any
blocking calls.

*/

/**
@fn TOBII_API tobii_error_t TOBII_CALL tobii_presence_subscribe( tobii_device_t* device, tobii_presence_callback_t callback, void* user_data );
@ingroup tobii_streams

tobii_presence_subscribe
-----------------------------

### Function

Start listening for presence notifications, reporting whether there is a person in front of the device.
The difference from the user_presence stream is that this signal is intended to be slower and detect
presence even if a user is turned away from the computer.


### Syntax

    #include <tobii/tobii_streams.h>
    tobii_error_t tobii_presence_subscribe( tobii_device_t* device,
        tobii_presence_callback_t callback, void* user_data );


### Remarks

This subscription is for being notified when a user is detected by the device, and when a user is no longer detected.

*device* must be a pointer to a valid tobii_device_t instance as created by calling tobii_device_create.

*callback* is a function pointer to a function with the prototype:

    void presence_callback( tobii_presence_t const* presence, void* user_data )


This function will be called when there is a change in presence state. It is called with the following parameters:

-   *presence*
        -   *status*
            One of the following values:

            -   **TOBII_PRESENCE_STATUS_UNKNOWN** if user presence could not be determined.
            -   **TOBII_PRESENCE_STATUS_AWAY** if there is a user in front of the device.
            -   **TOBII_PRESENCE_STATUS_PRESENT** if there is no user in front of the device.


        -   *timestamp_us*
            Timestamp value for when the user presence was calculated, measured in microseconds (us). The epoch is undefined,
            so these timestamps are only useful for calculating the time elapsed between a pair of values. The function
            tobii_system_clock() can be used to retrieve a timestamp using the same clock and same relative values as this
            timestamp.

-   *user_data*
    This is the custom pointer sent in when registering the callback.


*user_data* custom pointer which will be passed unmodified to the callback.


### Return value

If the operation is successful, tobii_presence_subscribe returns **TOBII_ERROR_NO_ERROR**. If the call
fails, tobii_presence_subscribe returns an error code specific to the device.

### See also

tobii_presence_unsubscribe(), tobii_device_process_callbacks(), tobii_system_clock()


### Example

@code{.c}

    #include <tobii/tobii_internal.h>
    #include <stdio.h>
    #include <assert.h>

    void presence_callback( tobii_presence_t const* presence, void* user_data )
    {
        switch( presence->status )
        {
            case TOBII_PRESENCE_STATUS_UNKNOWN:
                printf( "User presence status is unknown.\n" );
                break;
            case TOBII_PRESENCE_STATUS_AWAY:
                printf( "User is away.\n" );
                break;
            case TOBII_PRESENCE_STATUS_PRESENT:
                printf( "User is present.\n" );
                break;
        }
    }

    static void url_receiver( char const* url, void* user_data )
    {
        char* buffer = (char*)user_data;
        if( *buffer != '\0' ) return; // only keep first value

        if( strlen( url ) < 256 )
            strcpy( buffer, url );
    }

    int main()
    {
        tobii_api_t* api;
        tobii_error_t error = tobii_api_create( &api, NULL, NULL );
        assert( error == TOBII_ERROR_NO_ERROR );

        char url[ 256 ] = { 0 };
        error = tobii_enumerate_local_device_urls( api, url_receiver, url );
        assert( error == TOBII_ERROR_NO_ERROR && *url != '\0' );

        tobii_device_t* device;
        error = tobii_device_create( api, url, TOBII_FIELD_OF_USE_STORE_OR_TRANSFER_FALSE, &device );
        assert( error == TOBII_ERROR_NO_ERROR );

        error = tobii_presence_subscribe( device, presence_callback, 0 );
        assert( error == TOBII_ERROR_NO_ERROR );

        int is_running = 1000; // in this sample, exit after some iterations
        while( --is_running > 0 )
        {
            error = tobii_wait_for_callbacks( 1, &device );
            assert( error == TOBII_ERROR_NO_ERROR || error == TOBII_ERROR_TIMED_OUT );

            error = tobii_device_process_callbacks( device );
            assert( error == TOBII_ERROR_NO_ERROR );
        }

        error = tobii_presence_unsubscribe( device );
        assert( error == TOBII_ERROR_NO_ERROR );

        error = tobii_device_destroy( device );
        assert( error == TOBII_ERROR_NO_ERROR );

        error = tobii_api_destroy( api );
        assert( error == TOBII_ERROR_NO_ERROR );
        return 0;
    }

@endcode

*/
/**
@fn TOBII_API tobii_error_t TOBII_CALL tobii_low_frequency_head_position_subscribe( tobii_device_t* device, tobii_head_position_t callback, void* user_data );
@ingroup tobii_internal

tobii_low_frequency_head_position_subscribe
-------------------------------------------

### Function

Start listening to the low frequency head position stream, which reports the position of the user's head at a low
frequency.


### Syntax

    #include <tobii/tobii_internal.h>
    tobii_error_t tobii_low_frequency_head_position_subscribe( tobii_device_t* device,
        tobii_head_position_t callback, void* user_data );


### Remarks

To be able to call this function, the *device* should have been created with a minimum license level of Internal
feature group.

*device* must be a pointer to a valid tobii_device_t as created by calling tobii_device_create.

*callback* is a function pointer to a function with the prototype:

    void head_position_callback( tobii_head_position_t const* head_position, void* user_data )

This function will be called when there is new head position data to be sent to the subscriber. It is called with the
following parameters:

-   *head_position*

    This is a pointer to a struct containing the following data:

    -   *timestamp_us*

        Timestamp value for when the head pose was calculated, measured in microseconds (us). The epoch is
        undefined, so these timestamps are only useful for calculating the time elapsed between a pair of values.
        The function tobii_system_clock() can be used to retrieve a timestamp using the same clock and same relative
        values as this timestamp.

    -   *validity*

        Indicates the validity of the *position_xyz* field. **TOBII_VALIDITY_INVALID** if the field is not valid,
        **TOBII_VALIDITY_VALID** if it is.

    -   *position_xyz*

        An array of three floats, for the x, y and z coordinate of the head of the user, as measured in millimeters
        from the center of the display.


-   *user_data*

    This is the custom pointer sent in when registering the callback.

*user_data* custom pointer which will be passed unmodified to the callback.


### Return value

If the call was successful **TOBII_ERROR_NO_ERROR** will be returned. If the call has failed an error
code specific to the device will be returned.

### See Also

tobii_low_frequency_head_position_unsubscribe()

*/

/**
@fn TOBII_API tobii_error_t TOBII_CALL tobii_low_frequency_head_position_unsubscribe( tobii_device_t* device );
@ingroup tobii_internal

tobii_low_frequency_head_position_unsubscribe
---------------------------------------------

### Function

Stops listening to low frequency head position stream that was subscribed to by a call to
tobii_low_frequency_head_position_subscribe()


### Syntax

    #include <tobii/tobii_internal.h>
    tobii_error_t tobii_low_frequency_head_position_unsubscribe( tobii_device_t* device );


### Remarks

To be able to call this function, the *device* should have been created with a minimum license level of Internal
feature group.

*device* must be a pointer to a valid tobii_device_t as created by calling tobii_device_create.


### Return value

If the call was successful **TOBII_ERROR_NO_ERROR** will be returned. If the call has failed an error
code specific to the device will be returned.

### See Also

tobii_low_frequency_head_position_subscribe()

*/

