#addin nuget:?package=Cake.CMake

var target = Argument("target","BuildEditor");

Task("CMakeBuild")
.Does(
    ()=>{
        var settings = new CMakeSettings
        {
            SourcePath = "./src",
            OutputPath = "./build"
        };
        CMake(settings);  
    }
);
Task("BuildRuntime")
// .IsDependentOn("CMakeBuild")
.Does(
    ()=>{
        // MSBuild("./Editor/ToyStation.Runtime/ToyStation.Runtime.csproj",new MSBuildSettings{
        //     Verbosity = Verbosity.Verbose,
        //     ToolVersion = MSBuildToolVersion.VS2022,
        //     Configuration = "Debug",
        //     PlatformTarget = PlatformTarget.MSIL
        // });
        DotNetBuild("./src/Editor/ToyStation.Runtime/ToyStation.Runtime.csproj",new DotNetBuildSettings{
        });
    }
);
Task("BuildVulkan")
.IsDependentOn("BuildRuntime")
.Does(
    ()=>{
        DotNetBuild("./src/Editor/ToyStation.Vulkan/ToyStation.Vulkan.csproj",new DotNetBuildSettings{
        });
    }
);

Task("BuildEditor")
.IsDependentOn("BuildVulkan")
.Does(
    ()=>{
        DotNetBuild("./src/Editor/Editor/Editor.csproj",new DotNetBuildSettings{
        });
    }
);

RunTarget(target);