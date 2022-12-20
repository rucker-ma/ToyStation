#addin nuget:?package=Cake.CMake&version=1.3.1

var target = Argument("target","BuildEditor");

Task("CMakeGenerate")
.Does(
    ()=>{
        var settings = new CMakeSettings
        {
            SourcePath = "./",
            OutputPath = "./build"
        };
        CMake(settings);

    }
);
Task("CMakeBuild")
.IsDependentOn("CMakeGenerate")
.Does(
    ()=>{
        var settings = new CMakeBuildSettings
        {
            BinaryPath = "./build"
        };
        CMakeBuild(settings);
    }
);
Task("BuildRuntime")
.IsDependentOn("CMakeBuild")
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