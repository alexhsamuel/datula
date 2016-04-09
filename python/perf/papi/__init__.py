"""
PAPI (Performance API) library wrappers.

See the [PAPI wiki](http://icl.cs.utk.edu/projects/papi/wiki/Main_Page) for
details.
"""

#-------------------------------------------------------------------------------

import ctypes
import itertools

from   . import api

#-------------------------------------------------------------------------------

def _check(value):
    if value < api.PAPI_OK:
        raise api.Error(value)
    else:
        return value
    

def initialize(version=api.PAPI_VER_CURRENT):
    api._import()
    if not api.PAPI_is_initialized():
        lib_version = api.PAPI_library_init(version)
        if lib_version == version:
            return
        elif lib_version < 0:
            raise RuntimeError("failed to initialize PAPI")
        else:
            raise RuntimeError(
                "version mismatch: expected {}, got {}".format(
                    ".".join(api.PAPI_UNPACK_VERSION_NUMBER(version)),
                    ".".join(api.PAPI_UNPACK_VERSION_NUMBER(lib_version))))
        

def get_events(sort):
    initialize()
    if sort == "native":
        code = api.PAPI_NATIVE_MASK
    elif sort == "preset":
        code = api.PAPI_PRESET_MASK
    else:
        raise ValueError("unknown sort: {!r}".format(sort))

    code = ctypes.c_int(code)
    while True:
        if api.PAPI_query_event(code) == api.PAPI_OK:
            info = api.EventInfo()
            _check(api.PAPI_get_event_info(code, info))
            yield info

        if api.PAPI_enum_event(ctypes.byref(code), api.PAPI_ENUM_ALL) != api.PAPI_OK:
            break


def find_event_by_symbol(symbol):
    for event in itertools.chain(get_events("native"), get_events("preset")):
        if event.symbol.decode() == symbol:
            return event
    else:
        raise NameError(symbol)


class Counters:

    def __init__(self, *events):
        # Look up event names from strings, if needed.
        events = tuple(
            e if isinstance(e, api.EventInfo) else find_event_by_symbol(str(e))
            for e in events
        )

        self.__events = events
        num = len(self.__events)
        self.__num = ctypes.c_int(num)
        self.__event_codes = (ctypes.c_int * num)(*( e.event_code for e in self.__events ))
        self.__names = tuple( e.symbol.decode() for e in events )
        self.__counters = (ctypes.c_longlong * num)()
        self.__totals = (0, ) * len(self.__events)


    def __enter__(self):
        _check(api.PAPI_start_counters(self.__event_codes, self.__num))
        return self


    def __exit__(self, *exc_info):
        _check(api.PAPI_stop_counters(self.__counters, self.__num))
        self.__totals = tuple(
            c0 + c1 for c0, c1 in zip(self.__totals, self.__counters) )


    @property
    def counters(self):
        return dict(zip(self.__names, self.__totals))


    def __str__(self):
        return " ".join(
            "{}={}".format(n, t) for n, t in zip(self.__names, self.__totals) )



