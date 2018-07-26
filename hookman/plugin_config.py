import ctypes
import sys
from pathlib import Path
from typing import List
from zipfile import ZipFile

import attr
from attr import attrib

from hookman.exceptions import PluginNotFoundError
from hookman.hookman_utils import load_shared_lib

@attr.s(frozen=True, auto_attribs=True)
class ConflictStatus(object):
    """
    Class that hold information related to conflict between plugins.
    """
    plugins : List[str]
    hook : str

@attr.s(frozen=True)
class PluginInfo(object):
    """
    Class that holds all information related to the plugin with some auxiliary methods
    """
    location = attrib(type=Path)
    hooks_available = attrib(type=dict)

    author = attrib(type=str, init=False)
    description = attrib(type=str, default="Could not find a description", init=False)
    email = attrib(type=str, init=False)
    hooks_implemented = attrib(type=list, init=False)
    name = attrib(type=str, init=False)
    shared_lib_name = attrib(type=str, init=False)
    shared_lib_path = attrib(type=Path, init=False)
    version = attrib(type=str, init=False)

    def __attrs_post_init__(self):
        plugin_config_file_content = self._load_yaml_file(self.location.read_text(encoding="utf-8"))

        object.__setattr__(self, "author", plugin_config_file_content['author'])
        object.__setattr__(self, "email", plugin_config_file_content['email'])
        object.__setattr__(self, "name", plugin_config_file_content['plugin_name'])
        object.__setattr__(self, "shared_lib_name", plugin_config_file_content['shared_lib_name'])
        object.__setattr__(self, "shared_lib_path", self.location.parent / self.shared_lib_name)
        object.__setattr__(self, "version", plugin_config_file_content['plugin_version'])
        object.__setattr__(self, "hooks_implemented", self._get_hooks_implemented())

        readme_file = self.location.parent / 'readme.md'
        if readme_file.exists():
            object.__setattr__(self, "description", readme_file.read_text())


    def _get_hooks_implemented(self) -> List[str]:
        """
        Return a list of which hooks from "hooks_available" the shared library implements
        """
        with load_shared_lib(str(self.shared_lib_path)) as plugin_dll:
            hooks_implemented = [
                hook_name
                for hook_name, full_hook_name in self.hooks_available.items()
                if PluginInfo.is_implemented_on_plugin(plugin_dll, full_hook_name)
            ]
        return hooks_implemented


    @classmethod
    def is_implemented_on_plugin(cls, plugin_dll: ctypes.CDLL, hook_name: str) -> bool:
        """
        Check if the given function name is available on the plugin_dll informed

        .. note:: The hook_name should be the full name of the hook
        Ex.: {project}_{version}_{hook_name} -> hookman_v4_friction_factor
        """
        try:
            getattr(plugin_dll, hook_name)
        except AttributeError:
            return False

        return True

    @classmethod
    def _load_yaml_file(cls, yaml_content):
        import strictyaml
        schema = strictyaml.Map({
            "plugin_name": strictyaml.Str(),
            "plugin_version": strictyaml.Str(),
            "author": strictyaml.Str(),
            "email": strictyaml.Str(),
            "shared_lib": strictyaml.Str(),
        })
        plugin_config_file_content = strictyaml.load(yaml_content, schema).data
        if sys.platform == 'win32':
            plugin_config_file_content[
                'shared_lib_name'] = f"{plugin_config_file_content['shared_lib']}.dll"
        else:
            plugin_config_file_content[
                'shared_lib_name'] = f"lib{plugin_config_file_content['shared_lib']}.so"
        return plugin_config_file_content

    @classmethod
    def validate_plugin_file(cls, plugin_file_zip: ZipFile):
        """
        Check if the given plugin_file is valid,
        currently the only check that this method do is to verify if the shared_lib is available
        """
        list_of_files = [file.filename for file in plugin_file_zip.filelist]

        plugin_file_str = plugin_file_zip.read('plugin.yaml').decode("utf-8")
        plugin_file_content = PluginInfo._load_yaml_file(plugin_file_str)

        if plugin_file_content['shared_lib_name'] not in list_of_files:
            raise PluginNotFoundError(
                f"{plugin_file_content['shared_lib_name']} could not be found "
                f"inside the plugin file")


    @classmethod
    def get_function_address(cls, plugin_dll, hook_name):
        """
        Return the address of the requested hook for the given plugin_dll.

        .. note:: The hook_name should be the full name of the hook
        Ex.: {project}_{version}_{hook_name} -> hookman_v4_friction_factor
        """
        return ctypes.cast(getattr(plugin_dll, hook_name), ctypes.c_void_p).value
