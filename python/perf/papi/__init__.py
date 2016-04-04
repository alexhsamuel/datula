"""
PAPI (Performance API) library wrappers.

See the [PAPI wiki](http://icl.cs.utk.edu/projects/papi/wiki/Main_Page) for
details.
"""

# FIXME: It's rude to import the shared library (which might not be installed!)
# at import time.

#-------------------------------------------------------------------------------

import ctypes

#-------------------------------------------------------------------------------

try:
    _lib = ctypes.CDLL("libpapi.so")
except OSError:
    raise RuntimeError("can't import libpapi.so; please install it")

#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------

def PAPI_VERSION_NUMBER(maj, min, rev, inc):
    return ((((maj << 24) | (min << 16)) | (rev << 8)) | inc)

def PAPI_VERSION_MAJOR(x):
    return ((x >> 24) & 255)

def PAPI_VERSION_MINOR(x):
    return ((x >> 16) & 255)

def PAPI_VERSION_REVISION(x):
    return ((x >> 8) & 255)

def PAPI_VERSION_INCREMENT(x):
    return (x & 255)

def PAPI_UNPACK_VERSION_NUMBER(ver):
    return (ver >> 24, ver >> 16 & 0xff, var >> 8 & 0xff, ver & 0xff)

PAPI_VERSION = PAPI_VERSION_NUMBER(5, 4, 1, 0)
PAPI_VER_CURRENT = PAPI_VERSION & 0xffff0000

PAPI_OK = 0
PAPI_PRESET_MASK = 0x80000000
PAPI_NATIVE_MASK = 0x40000000

PAPI_ENUM_EVENTS = 0
PAPI_ENUM_ALL = PAPI_ENUM_EVENTS

#-------------------------------------------------------------------------------
# Types
#-------------------------------------------------------------------------------

class Error(RuntimeError):
    """
    Exception mapping PAPI error codes to human-readable strings.
    """

    def __init__(self, code):
        super.__init__(
            "PAPI error code {} ({})".format(code, PAPI_strerror(code)))
        self.code = code


    def __repr__(self):
        return "{}({})".format(self.__class__.__name__, self.code)



class EventInfo(ctypes.Structure):

    _fields_ = [
        ('event_code', ctypes.c_uint),
        ('symbol', ctypes.c_char * 1024),
        ('short_descr', ctypes.c_char * 64),
        ('long_descr', ctypes.c_char * 1024),
        ('component_index', ctypes.c_int),
        ('units', ctypes.c_char * 64),
        ('location', ctypes.c_int),
        ('data_type', ctypes.c_int),
        ('value_type', ctypes.c_int),
        ('timescope', ctypes.c_int),
        ('update_type', ctypes.c_int),
        ('update_freq', ctypes.c_int),
        ('count', ctypes.c_uint),
        ('event_type', ctypes.c_uint),
        ('derived', ctypes.c_char * 64),
        ('postfix', ctypes.c_char * 256),
        ('code', ctypes.c_uint * 12),
        ('name', (ctypes.c_char * 256) * 12),
        ('note', ctypes.c_char * 1024),
    ]

    __slots__ = [ n for n, *_ in _fields_ ]

    def __repr__(self):
        return "{}({})".format(
            self.__class__.__name__,
            ", ".join(
                "{}={!r}".format(n, getattr(self, n))
                for n, *_ in self._fields_
            ))


#-------------------------------------------------------------------------------
# Functions
#-------------------------------------------------------------------------------

PAPI_is_initialized = _lib.PAPI_is_initialized
PAPI_is_initialized.argtypes = []
PAPI_is_initialized.restype = ctypes.c_int

PAPI_library_init = _lib.PAPI_library_init
PAPI_library_init.argtypes = [ctypes.c_int]
PAPI_library_init.restype = ctypes.c_int

PAPI_num_counters = _lib.PAPI_num_counters
PAPI_num_counters.argtypes = []
PAPI_num_counters.restype = ctypes.c_int

PAPI_num_components = _lib.PAPI_num_components
PAPI_num_components.argtypes = []
PAPI_num_components.restype = ctypes.c_int

PAPI_read_counters = _lib.PAPI_read_counters
PAPI_read_counters.argtypes = [ctypes.POINTER(ctypes.c_longlong), ctypes.c_int]
PAPI_read_counters.restype = ctypes.c_int

PAPI_start_counters = _lib.PAPI_start_counters
PAPI_start_counters.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
PAPI_start_counters.restype = ctypes.c_int

PAPI_stop_counters = _lib.PAPI_stop_counters
PAPI_stop_counters.argtypes = [ctypes.POINTER(ctypes.c_longlong), ctypes.c_int]
PAPI_stop_counters.restype = ctypes.c_int

PAPI_query_event = _lib.PAPI_query_event
PAPI_query_event.argtypes = [ctypes.c_int]
PAPI_query_event.restype = ctypes.c_int

PAPI_get_event_info = _lib.PAPI_get_event_info
PAPI_get_event_info.argtypes = [ctypes.c_int, ctypes.POINTER(EventInfo)]
PAPI_get_event_info.restype = ctypes.c_int

PAPI_enum_event = _lib.PAPI_enum_event
PAPI_enum_event.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
PAPI_enum_event.restype = ctypes.c_int

#-------------------------------------------------------------------------------

def _check(value):
    if value < PAPI_OK:
        raise PAPIError(value)
    else:
        return value
    

def _initialize(version=PAPI_VER_CURRENT):
    if not PAPI_is_initialized():
        lib_version = PAPI_library_init(version)
        if lib_version == version:
            return
        elif lib_version < 0:
            raise RuntimeError("failed to initialize PAPI")
        else:
            raise RuntimeError(
                "version mismatch: expected {}, got {}".format(
                    ".".join(PAPI_UNPACK_VERSION_NUMBER(version)),
                    ".".join(PAPI_UNPACK_VERSION_NUMBER(lib_version))))
        

_initialize()

def _get_events(sort):
    if sort == "native":
        code = PAPI_NATIVE_MASK
    elif sort == "preset":
        code = PAPI_PRESET_MASK
    else:
        raise ValueError("unknown sort: {!r}".format(sort))

    code = ctypes.c_int(code)
    while True:
        if PAPI_query_event(code) == PAPI_OK:
            info = EventInfo()
            _check(PAPI_get_event_info(code, info))
            yield info

        if PAPI_enum_event(ctypes.byref(code), PAPI_ENUM_ALL) != PAPI_OK:
            break


# FIXME: Don't like.
globals().update({ e.symbol.decode(): e for e in _get_events("native") })
globals().update({ e.symbol.decode(): e for e in _get_events("preset") })

