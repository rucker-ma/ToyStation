#pragma once
#include <string>
#include <filesystem>
#include  "helper.h"
#include "ClassCursor.h"

class FileParser {
public:
  FileParser(std::filesystem::path filepath, CXIndex index);
  ~FileParser();
  void Generate();
  void Parse(Cursor& root_cursor);
  std::vector<std::shared_ptr<Cursor>>& Cursors();
  std::filesystem::path Path();
private:
    void Verify();
private:
    std::filesystem::path file_path_;
  CXIndex idx_;
  CXTranslationUnit unit_;
  Cursor root_cursor_;
  std::vector<std::string> name_space;
  std::shared_ptr<MacrosDelegate> Del;
  std::vector<std::shared_ptr<Cursor>> output_cursors_;

  std::vector<const char*> arguments = {
    {
        "-x",
        "c++",        //�������ļ���Ϊc++�����ļ�                                  
        "-std=c++17", //��������Ա�׼            
        "-DNDEBUG",
        "-w", //���ƾ���
        // "-MG", //���ȱʧ��ͷ�ļ��������ļ���
        //"-M",  //ֻ����Ԥ����-E������ӡ����ͷ�ļ�������̨                        
        "-E",
        "-ferror-limit=0",                                       //�������Ҳ����ֹͣ
        // "-ID:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine", //-I�������ļ���
        // "-ID:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine/Render",
        // "-ID:/software/VulkanSDK/1.3.231.1/Include",
        // "-ID:/project/cpp/graphics/vk-demo/3rd/glm-0.9.9.8",
        "-o clangLog.txt" //д�����ָ���ļ�
    }
  };

};
