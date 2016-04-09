"""
Wrappers for raw PAPI API.
"""

#-------------------------------------------------------------------------------

import ctypes
from   ctypes import c_int, c_longlong, c_uint, c_char, c_char_p, POINTER

#-------------------------------------------------------------------------------
# Types
#-------------------------------------------------------------------------------

class Error(RuntimeError):
    """
    Exception mapping PAPI error codes to human-readable strings.
    """

    def __init__(self, code):
        super().__init__(
            "PAPI error code {} ({})".format(code, PAPI_strerror(code)))
        self.code = code


    def __repr__(self):
        return "{}({})".format(self.__class__.__name__, self.code)



class EventInfo(ctypes.Structure):

    _fields_ = [
        ("event_code", ctypes.c_uint),
        ("symbol", ctypes.c_char * 1024),
        ("short_descr", ctypes.c_char * 64),
        ("long_descr", ctypes.c_char * 1024),
        ("component_index", ctypes.c_int),
        ("units", ctypes.c_char * 64),
        ("location", ctypes.c_int),
        ("data_type", ctypes.c_int),
        ("value_type", ctypes.c_int),
        ("timescope", ctypes.c_int),
        ("update_type", ctypes.c_int),
        ("update_freq", ctypes.c_int),
        ("count", ctypes.c_uint),
        ("event_type", ctypes.c_uint),
        ("derived", ctypes.c_char * 64),
        ("postfix", ctypes.c_char * 256),
        ("code", ctypes.c_uint * 12),
        ("name", (ctypes.c_char * 256) * 12),
        ("note", ctypes.c_char * 1024),
    ]

    __slots__ = [ n for n, *_ in _fields_ ]

    def __repr__(self):
        return "{}({})".format(
            self.__class__.__name__,
            ", ".join(
                "{}={!r}".format(n, getattr(self, n))
                for n, _ in self._fields_
            ))


#-------------------------------------------------------------------------------
# Functions
#-------------------------------------------------------------------------------

_lib = None

_FUNCTIONS = (
    ("PAPI_is_initialized"  , [], c_int),
    ("PAPI_library_init"    , [c_int], c_int),
    ("PAPI_num_counters"    , [], c_int),
    ("PAPI_num_components"  , [], c_int),
    ("PAPI_read_counters"   , [POINTER(c_longlong), c_int], c_int),
    ("PAPI_start_counters"  , [POINTER(c_int), c_int], c_int),
    ("PAPI_stop_counters"   , [POINTER(c_longlong), c_int], c_int),
    ("PAPI_query_event"     , [c_int], c_int),
    ("PAPI_get_event_info"  , [c_int, POINTER(EventInfo)], c_int),
    ("PAPI_enum_event"      , [POINTER(c_int), c_int], c_int),
    ("PAPI_strerror"        , [c_int], c_char_p),
)


def _import():
    """
    Loads the PAPI shared library and imports API functions into the module.
    """
    global _lib
    if _lib is None:
        try:
            _lib = ctypes.CDLL("libpapi.so")
        except OSError:
            raise RuntimeError("can't import libpapi.so; please install it")

        for name, argtypes, restype in _FUNCTIONS:
            fn = getattr(_lib, name)
            fn.argtypes = argtypes
            fn.restype = restype
            globals()[name] = fn
        
    return _lib


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

