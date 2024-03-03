import abc
from typing import Optional
import dbm


class KVStore(abc.ABC):
    '''Interface for normal key-value store operations'''

    @abc.abstractmethod
    def exists(self, key: str) -> bool:
        '''raise NotImplementedError()'''

    @abc.abstractmethod
    def get(self, key: str, fallback: Optional[str] = None) -> str:
        '''raise NotImplementedError()'''

    @abc.abstractmethod
    def set(self, key: str, value: str) -> str:
        '''raise NotImplementedError()'''

    @abc.abstractmethod
    def pop(self, key: str) -> str:
        '''raise NotImplementedError()'''


class DBMStore(KVStore):

    def __init__(self, fpath):
        self._fpath = fpath

    def exists(self, key: str) -> bool:
        with dbm.open(self._fpath, 'c') as db:
            exists = True if key in db else False
        return exists

    def get(self, key: str, fallback: Optional[str] = None) -> str:
        with dbm.open(self._fpath, 'c') as db:
            try:
                value = db.get(key).decode()
            except AttributeError:
                value = fallback
        return value

    def set(self, key: str, value: str) -> str:
        with dbm.open(self._fpath, 'c') as db:
            db[key] = value
        return value

    def pop(self, key: str) -> str:
        with dbm.open(self._fpath, 'c') as db:
            value = db.get(key).decode()
            del db[key]
        return value
