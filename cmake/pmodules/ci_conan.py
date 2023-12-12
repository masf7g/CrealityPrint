import sys
import os
import platform
from pathlib import Path
from xml.etree import ElementTree as ET

class Conan():
    '''
    system      //Windows , Linux, Darwin
    cmake_path  //cmake module directory
    xml_file    //store the whole recipes
    whole_libs  //libs dict load from xml_file
    conan_path  //conan directory
    '''
    def __init__(self, cpath):
        self.system = platform.system()
        self.cmake_path = Path(cpath)   
        self.xml_file = self.cmake_path.joinpath("/conan/graph/libs.xml")
        self.conan_path = self.cmake_path.joinpath("/conan/")
        self.whole_libs = _create_whole_libs()
        
    '''
    load whole libs from conan/graph/libs.xml
    '''    
    def _create_whole_libs(self):
        tree = ET.parse(self.xml_file)
        root = tree.getroot()
        
        libs = root.findall("lib")
        subs = {}
        for lib in libs:
            sublibs = lib.findall("sublib")
            subss = []
            for sub in sublibs:
                subss.append(sub.attrib["name"])
            
            subs[lib.attrib["name"]] = subss
        return subs
        
    '''
    input patches, output libs chain 
    '''
    def _collect_sequece_libs(self, libDict, libs):
        result = []
        
        first = libs
        second = []
        while len(first) > 0:
            for value in first:
                nex = libDict[value]
                if len(nex) == 0:
                    result.append(value)
                else:
                    need = False
                    for nvalue in nex:
                        if nvalue not in result:
                            second.append(nvalue)
                            need = True
                    
                    if need == True:
                        second.append(value)
                    else:
                        result.append(value)
                    
            first = second
            second = []
        
        new_result = []
        for r in result:
            if r not in new_result:
                new_result.append(r)
            
        return new_result
    
    '''
    collect libs from a .txt file
    '''
    def _collect_libs_from_txt(self, file_name):
        libs = []
        with open(file_name, 'r') as file:
            contents = file.readlines()
            for content in contents:
                libs.append(content.rstrip())
                
            print("{0} load recipes: {1}".format(file_name, libs))
            file.close()    
        return libs
        
    '''
    collect libs from a root .txt file, 
    it will interate the children directory
    '''
    def _collect_libs_from_root_txt(self, graph_file):
        libs = _collect_libs_from_txt(Path(graph_file))
        children = Path(graph_file).parent.iterdir()
        for idx, element in enumerate(children):    
            if element.is_dir():
                child_graph_file = element.joinpath('graph.txt')
                #print("create_libs_from_txt -> load {0}".format(child_graph_file))
                
                if child_graph_file.exists() == True:
                    sub_libs = _collect_libs_from_txt(child_graph_file)
                    libs.extend(sub_libs)
            
        return libs
    
    '''
    upload conan package  recipe@channel
    '''
    def _conan_upload(self, recipe, channel):
        cmd = 'conan upload {}@{} -r artifactory --all -c'.format(recipe, channel)
        os.system(cmd)
    
    '''
    collect chain sub libs of one recipe
    '''
    def _collect_chain_sub_libs(self, recipe):
        result = []
        subs = libDict
        
        first = subs[recipe];
        second = []
        while len(first) > 0:
            for value in first:
                if value not in result:
                    result.append(value)
                    if value in subs:
                        nex = subs[value]
                        for nvalue in  nex:
                            if nvalue not in second:
                                second.append(nvalue)
                
            first = second
            second = []
        
        return result
    
    '''
    create one conan recipe package
    recipe xxx/x.x.x
    channel 
    '''
    def _create_one_conan_recipe(self, recipe, channel, upload):
        if recipe not in libDict:
            return
            
        segs = recipe.split('/')
        name = 'xxx'
        version = 'x.x.x'
        
        if len(segs) == 2:
            name = segs[0]
            version = segs[1]

        if '{}/{}'.format(name, version) not in libDict:
            return
            
        directory = str(self.conan_path)
        profile = self._profile()
        user_channel = channel
        subLibs = _collect_chain_sub_libs(recipe)
        
        with tempfile.TemporaryDirectory() as temp_directory:
            print("[conan DEBUG] created temporary directory ", temp_directory)
            print("[conan DEBUG] temp directory exist ", os.path.exists(temp_directory))
            
            create_script_src = directory + "/scripts/conanfile.py"
            cmake_script_src = directory + "/scripts/CMakeLists.txt"
            meta_data_src = directory + "/recipes/" + name + "/conandata.yml";
            shutil.copy2(create_script_src, temp_directory)
            shutil.copy2(meta_data_src, temp_directory)
            shutil.copy2(cmake_script_src, temp_directory)
            
            meta_data_dest = temp_directory + "/conandata.yml"
            meta_file = open(meta_data_dest, "a")
            if channel == 'jwin':
                user_channel = 'desktop/win'
                meta_file.write("generator: Ninja\n")
                
            meta_file.write("version: " + "\"" + version + "\"\n")
            meta_file.write("name: " + "\"" + name + "\"\n")
            meta_file.write("channel: " + "\"" + user_channel + "\"")
            meta_file.close()
            cmake_script_dest = temp_directory + "/CMakeLists.txt"
            cmake_file = open(cmake_script_dest, "a")
            cmake_file.write("add_subdirectory(" + name + ")")
            cmake_file.close()       
            sublibs_dest = temp_directory + "/requires.txt"
            subLibs_file = open(sublibs_dest, "w")
            for sub in subLibs:
                subLibs_file.write(sub)
                subLibs_file.write('\n')
            subLibs_file.close() 
            
            #os.system("pause")
            debug_cmd = 'conan create --profile {} -s build_type=Debug {} {}'.format(profile, temp_directory, user_channel)
            os.system(debug_cmd)
            release_cmd = 'conan create --profile {} -s build_type=Release {} {}'.format(profile, temp_directory, user_channel)        
            os.system(release_cmd)
        
        if upload == True:
            conan_upload(recipe, channel)
            
    '''
    create one conan recipe package
    recipe xxx/x.x.x
    channel 
    '''
    def _create_conan_recipes(self, recipes, channel, upload):
        for recipe in recipes:
            _create_one_conan_recipe(self, recipe, channel, upload)
    
    '''
    type [linux mac win opensource-mac opensource-win opensource-linux]
    '''
    def _channel(self, channel_name='desktop'):
        '''
        if name == 'linux':
            channel = 'desktop/linux'
        if name == 'mac':
            channel = 'desktop/mac'
        if name == 'opensource-linux':
            channel = 'opensource/linux'
        if name == 'opensource-mac':
            channel = 'opensource/mac'  
        if name == 'opensource-win':
            channel = 'opensource/win' 
        if name == 'jwin':
            channel = 'jwin'
            
        '''
        prof = self._profile()
        prefix = channel_name
            
        channel = '{}/{}'.format(prefix, prof)
        return channel
        
    def _profile(self):
        '''
        profile = 'win'
        if name == 'linux' or name == 'opensource-linux':
            profile = 'linux'
        if name == 'mac' or name == 'opensource-mac':
            profile = 'mac'
        return profile           
        '''
        if self.system == 'Windows':
            return 'win'
        if self.system == 'Linux':
            return 'linux'
        if self.system == 'Darwin':
            return 'mac'
        return 'win'    
        
    '''
    api , create from one, patches, subs, whole
    '''
    def create_from_patch_file(self, file_name, channel_name, upload):
        subs = self._collect_libs_from_txt(file_name)
        self._create_conan_recipes(subs, self._channel(channel_name), upload)

    def create_from_subs_file(self, file_name, channel_name, upload):
        subs = self._collect_libs_from_txt(file_name)
        seq_subs = self._collect_sequece_libs(libDict, subs)
        self._create_conan_recipes(seq_subs, self._channel(channel_name), upload)
        
    def create_one(self, recipe, channel_name, upload):
        libs = []
        libs.append(recipe)
        self._create_conan_recipes(libs, self._channel(channel_name), upload)
        
    def create_whole(self, channel_name, upload):
        libs = self._collect_sequece_libs(libDict, libDict.keys())
        self._create_conan_recipes(libs, self._channel(channel_name), upload)