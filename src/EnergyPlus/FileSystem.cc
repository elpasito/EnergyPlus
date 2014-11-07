//Standard C++ library
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// EnergyPlus Headers
#include <FileSystem.hh>
#include <DataStringGlobals.hh>
#include <DisplayRoutines.hh>


namespace EnergyPlus{

namespace FileSystem {

using namespace DataStringGlobals;

#ifdef _WIN32
std::string const exeExtension(".exe");
#else
std::string const exeExtension("");
#endif

void
makeNativePath(std::string &path)
{
	std::replace(path.begin(), path.end(), altpathChar, pathChar);
}

std::string
getFileName( std::string const& filePath )
{
	int pathCharPosition = filePath.find_last_of(pathChar);
	return filePath.substr(pathCharPosition + 1, filePath.size() - 1);
}

std::string 
getParentDirectoryPath( std::string const& path )
{
	std::string tempPath = path;
	if (path.at(path.size()-1) == pathChar)
		tempPath = path.substr(0, path.size()-1);

	int pathCharPosition = tempPath.find_last_of(pathChar);
	tempPath = tempPath.substr(0, pathCharPosition + 1);

	if (tempPath == "")
		tempPath = ".";

	return tempPath;
}

std::string
getAbsolutePath( std::string const& path )
{

#ifdef _WIN32
	char absolutePath[1024];
	GetFullPathName(path.c_str(), sizeof(absolutePath), absolutePath, NULL);
	return std::string(absolutePath);
#else
	// If the path doesn't exist, find which of it's parents' paths does exist
	std::string parentPath = path;
	while(!pathExists(parentPath)) {
		parentPath = getParentDirectoryPath(parentPath);
	}

	std::string pathTail;
	if ( parentPath == "." )
		pathTail = pathChar + path;
	else
		pathTail = pathChar + path.substr(parentPath.size(), path.size() - parentPath.size());

	char *absolutePathTemp = realpath(parentPath.c_str(), NULL);
	if (absolutePathTemp != NULL) {
		std::string absoluteParentPath(absolutePathTemp);
	    free(absolutePathTemp);
		return absoluteParentPath + pathTail;
	}
	else {
		DisplayString("ERROR: Could not resolve path for " + path + ".");
		exit(EXIT_FAILURE);
	}
#endif


}

std::string
getProgramPath()
{
	char executableRelativePath[1024];

#ifdef __APPLE__
	uint32_t pathSize = sizeof(executableRelativePath);
	_NSGetExecutablePath(executableRelativePath, &pathSize);
#elif __linux__
	ssize_t len = readlink("/proc/self/exe", executableRelativePath, sizeof(executableRelativePath)-1);
#elif _WIN32
	GetModuleFileName(NULL, executableRelativePath, sizeof(executableRelativePath));
#endif

	return std::string(executableRelativePath);
}

std::string
getFileExtension(const std::string& fileName){
	int extensionPosition = fileName.find_last_of(".");
	return fileName.substr(extensionPosition + 1, fileName.size() - 1);
}

std::string
removeFileExtension(const std::string& fileName){
	int extensionPosition = fileName.find_last_of(".");
	return fileName.substr(0, extensionPosition);
}

void
makeDirectory(std::string directoryPath)
{
	// Create a directory if doesn't already exist
	if ( pathExists(directoryPath) ){ // path already exists
		if ( !(directoryExists(directoryPath)) )
		{
			DisplayString("ERROR: " + getAbsolutePath(directoryPath) + " is not a directory.");
			exit(EXIT_FAILURE);
		}
	}
	else { // directory does not already exist
		std::string parentDirectoryPath = getParentDirectoryPath(directoryPath);
		if (!pathExists(parentDirectoryPath))
		{
			DisplayString("ERROR: " + getAbsolutePath(parentDirectoryPath) + " is not a directory.");
			exit(EXIT_FAILURE);
		}
#ifdef _WIN32
		CreateDirectory(directoryPath.c_str(), NULL);
#else
		mkdir(directoryPath.c_str(), 0755);
#endif
	}
}

bool
pathExists(std::string path)
{
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

bool
directoryExists(std::string directoryPath)
{
	struct stat info;
	if ( stat(directoryPath.c_str(), &info) == 0){
		return (info.st_mode & S_IFDIR);
	}
	else
		return false;
}

bool
fileExists(std::string filePath)
{
	struct stat info;
	if ( stat(filePath.c_str(), &info) == 0){
		return !(info.st_mode & S_IFDIR);
	}
	else
		return false;
}

void
moveFile(std::string filePath, std::string destination){
	rename(filePath.c_str(), destination.c_str());
}

int
systemCall(std::string command)
{
	return system(command.c_str());
}

void
removeFile(std::string fileName)
{
	remove(fileName.c_str());
}

void
linkFile(std::string fileName, std::string link)
{
#ifdef _WIN32
	CopyFile(fileName.c_str(), link.c_str(), false);
#else
	int status = symlink(fileName.c_str(), link.c_str());
#endif
}

}
}
