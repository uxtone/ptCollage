
#include <pxMem.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY PKEY_Device_FriendlyName = {{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};

#include "./pxwXAudio2.h"

pxwXAudio2::pxwXAudio2()
{
	_b_init       = false;
	_xa2          = NULL ;
	_voice_master = NULL ;
	_units        = NULL ;
	_unit_num     =     0;
	_stream       = NULL ;

	memset( &_device_detail, 0, sizeof(_device_detail) );
}

pxwXAudio2::~pxwXAudio2()
{
	_release();
}

bool pxwXAudio2::init   ( int32_t unit_num )
{
#ifndef pxwXAUDIO2_ENABLE
	// XAudio2Create is an undefined reference on this toolchain, can't deal
	return true;
#endif

	bool   b_ret      = false;
	UINT32 device_num =     0;

	if( _b_init ) return false;

#ifdef pxwXAUDIO2_ENABLE
	if( FAILED( XAudio2Create(&_xa2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) ) goto term;
#endif

	// update for new XAudio2 device enumeration behavior
	// if( FAILED( _xa2->GetDeviceCount( &device_num ) ) ) goto term;

	static IMMDeviceEnumerator* enumerator;
	enumerator = nullptr;
	if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator) != S_OK) goto term;

	static bool use_default; // TODO: hook up
	use_default = true;

	static IMMDevice* device;
	device = nullptr;

	if(use_default) {
		if (enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device) != S_OK) goto term;
	} else {
		static IMMDeviceCollection* collection;
		collection = nullptr;
		if (enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection) != S_OK) goto term;
		collection->GetCount(&device_num);

		if( device_num <= 0 ) goto term;
		collection->Item(0, &device);
	}

	//if( FAILED( _xa2->GetDeviceDetails( 0, &_device_detail ) ) ) goto term;

	static IPropertyStore* property_store;
	property_store = nullptr;
	if (FAILED(device->OpenPropertyStore(STGM_READ, &property_store))) goto term;

	static PROPVARIANT property;
	property = {};
	PropVariantInit(&property);
	if (SUCCEEDED(property_store->GetValue(PKEY_Device_FriendlyName, &property)))
	{
		memset(_device_detail.DisplayName, 0x00, sizeof(_device_detail.DisplayName));
		memcpy(_device_detail.DisplayName, property.pwszVal, lstrlenW(property.pwszVal) * sizeof(WCHAR));        PropVariantClear(&property);
	}
	{
		LPWSTR device_id = nullptr;
		if(FAILED(device->GetId(&device_id))) goto term;
		lstrcpynW(_device_detail.DeviceID, device_id, sizeof(_device_detail.DeviceID) / sizeof(WCHAR));
		CoTaskMemFree(device_id);
	}
	// if (SUCCEEDED(property_store->GetValue(PKEY_Device_InstanceId, &property)))
	// {
	//     memset(_device_detail.DeviceID, 0x00, sizeof(_device_detail.DisplayName));
	//     memcpy(_device_detail.DeviceID, property.pwszVal, wcslen(property.pwszVal));
	//     PropVariantClear(&property);
	// }
	_device_detail.Role = DefaultMultimediaDevice;

	static IAudioClient *client;
	client = nullptr;
	if (FAILED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void **>(&client)))) goto term;

	static WAVEFORMATEX *format;
	if (FAILED(client->GetMixFormat(&format))) goto term;
	if(format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		memcpy(&_device_detail.OutputFormat, format, sizeof(_device_detail.OutputFormat));
	}
	// end surgery

	if( FAILED( _xa2->CreateMasteringVoice( &_voice_master, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL ) ) ) goto term;

	if( !pxMem_zero_alloc( (void**)(&_units), sizeof(pxwXA2unit*) * unit_num ) ) goto term;
	_unit_num = unit_num;

	_b_init = true;
	b_ret   = true;
term:
	if( !b_ret ) _release();
	return b_ret;
}

void* pxwXAudio2::get_audio()
{
	if( !_b_init ) return NULL;
	return _xa2;
}

bool pxwXAudio2::_release()
{
	if( !_b_init ) return false;
	_b_init = false;

	SAFE_DELETE( _stream );

	if( _units )
	{
		for( int i = 0; i < _unit_num; i++ ) SAFE_DELETE( _units[ i ] );
		free( _units );
		_units = NULL;
	}

	if( _voice_master ) _voice_master->DestroyVoice(); _voice_master = NULL;
	if( _xa2 ) _xa2->Release(); _xa2 = NULL;

	return true;
}


bool pxwXAudio2::stream_init( int32_t ch_num, int32_t sps, float buf_sec )
{
	if( !_b_init ) return false;
	bool b_ret = false;
	if( !(_stream = new pxwXA2stream()) || !_stream->init( _xa2, ch_num, sps, buf_sec ) ) goto term;
	b_ret = true;
term:
	return b_ret;
}

bool pxwXAudio2::stream_start     ( pxfunc_sample_pcm sampling_proc, void* sampling_user, int priority, const char* th_name )
{
	if( !_b_init || !_stream ) return false;
	return _stream->start( sampling_proc, sampling_user, priority, th_name );
}

bool pxwXAudio2::stream_order_stop()
{
	if( !_b_init || !_stream ) return false;
	return _stream->order_stop();
}

bool pxwXAudio2::stream_is_working()
{
	if( !_b_init || !_stream ) return false;
	return _stream->is_working();
}

bool pxwXAudio2::stream_join( float timeout_sec, bool* p_b_success )
{
	if( !_b_init || !_stream ) return false;
	return _stream->join( timeout_sec, p_b_success );
}


bool pxwXAudio2::unit_create_16bps( int32_t no, int32_t ch_num, int32_t sps, const void* p_src, int32_t smp_num )
{
	bool b_ret = false;
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	if(    _units[ no ]                       ) return false;

	if( !( _units[ no ] = new pxwXA2unit() )  ) goto term;
	if( !  _units[ no ]->create_16bps( _xa2, ch_num, sps, p_src, smp_num ) ) goto term;
	b_ret = true;
term:
	if( !b_ret ) unit_release( no );

	return b_ret;
}

bool pxwXAudio2::unit_release( int32_t no )
{
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	SAFE_DELETE( _units[ no ] );
	return true;
}

bool pxwXAudio2::unit_play   ( int32_t no )
{
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	return _units[ no ]->play( false );
}

bool pxwXAudio2::unit_is_used( int32_t no ) const
{
	if( !_b_init ) return false;
	if( no < 0 || no >= _unit_num ) return false;
	if( !_units[ no ] ) return false;
	return true;
}


bool pxwXAudio2::unit_loop   ( int32_t no )
{
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	return _units[ no ]->play( true );
}

bool pxwXAudio2::unit_stop   ( int32_t no, bool b_force )
{
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	return _units[ no ]->stop( b_force );
}

bool pxwXAudio2::unit_volume ( int32_t no, float v )
{
	if( !_b_init || no < 0 || no >= _unit_num ) return false;
	return _units[ no ]->volume( v );
}
