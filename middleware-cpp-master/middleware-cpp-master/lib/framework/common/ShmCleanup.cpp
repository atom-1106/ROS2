// Copyright (C) Caterpillar Inc. All Rights Reserved.
// File: ShmCleanup.h
// Description: Cleans up things in shared memory left behind by Fast-DDS when not torn down.

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <ranges>
#include <string>

using namespace std::placeholders;

static bool CheckLockLockfile(std::string const& path, int& fd)
{
	fd = open(path.c_str(), O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		return false;
	}
	if(flock(fd, LOCK_EX | LOCK_NB) < 0)
	{
		close(fd);
		fd = -1;
		return false;
	}
	return true;
}

static void CleanUpLockfile(int& fd)
{
	// have to check because they will be -1 if that lock was not present
	if(fd != -1)
	{
		// release lock just in case a process was forked
		flock(fd, LOCK_UN);
		close(fd);
		fd = -1;
	}
}

static bool CheckLockAll(std::string const& baseFile, int& fd, int& el, int& sl)
{
	fd                 = -1;
	el                 = -1;
	sl                 = -1;
	std::string elPath = baseFile + "_el";
	if(std::filesystem::exists(elPath) && !CheckLockLockfile(elPath, el))
	{
		return false;
	}
	std::string slPath = baseFile + "_sl";
	if(std::filesystem::exists(slPath) && !CheckLockLockfile(slPath, sl))
	{
		CleanUpLockfile(el);
		return false;
	}
	fd = open(baseFile.c_str(), O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		CleanUpLockfile(el);
		CleanUpLockfile(sl);
		return false;
	}
	// man 2 fcntl
	// "A write lease may be placed on a file only if there are no other open file descriptors for the file."
	// do as well to be extra sure that the file is stale
	if(fcntl(fd, F_SETLEASE, F_WRLCK) < 0)
	{
		CleanUpLockfile(el);
		CleanUpLockfile(sl);
		close(fd);
		return false;
	}
	return true;
}

std::string TransformBaseFile(std::filesystem::directory_entry const& dirent, std::string const& dir)
{
	return dir + "/" + dirent.path().filename().string();
}

static void CleanPorts(std::string const& dir)
{
	auto const filterPortFiles = [](auto const& dirent)
	{
		std::string const name = dirent.path().filename().string();
		if(name.rfind("fastdds_port", 0) == std::string::npos && name.rfind("fastrtps_port", 0) == std::string::npos)
		{
			return false;
		}
		// skip _el exclusive lock / _sl shared lock files
		// we check from the port file because it always exists
		// (sometimes neither the _el or _sl exist)
		if(name.rfind("_") == name.size() - 3)
		{
			return false;
		}
		return true;
	};
	for(std::string const& baseFile : std::filesystem::recursive_directory_iterator(dir) |
	                                      std::views::filter(filterPortFiles) |
	                                      std::views::transform(std::bind(TransformBaseFile, _1, dir)))
	{
		int fd, el, sl;
		if(CheckLockAll(baseFile, fd, el, sl))
		{
			std::remove((baseFile).c_str());
			std::remove((baseFile + "_el").c_str());
			std::remove((baseFile + "_sl").c_str());
			CleanUpLockfile(el);
			CleanUpLockfile(sl);
			fcntl(fd, F_SETLEASE, F_UNLCK);
			close(fd);
		}
	}
}

static void CleanSegments(std::string const& dir)
{
	auto const filterSegments = [](auto const& dirent)
	{
		std::string const name = dirent.path().filename().string();
		if(name.rfind("fastdds_", 0) == std::string::npos && name.rfind("fastrtps_", 0) == std::string::npos)
		{
			return false;
		}
		// https://fast-dds.docs.eprosima.com/en/latest/fastdds/transport/shared_memory/shared_memory.html
		// "16-character UUID"
		if(name.size() - name.find("_") != 16)
		{
			return false;
		}
		return true;
	};
	for(std::string const& baseFile : std::filesystem::recursive_directory_iterator(dir) |
	                                      std::views::filter(filterSegments) |
	                                      std::views::transform(std::bind(TransformBaseFile, _1, dir)))
	{
		// only _el seems to ever exist, but this can go through the same process
		int fd, el, sl;
		if(CheckLockAll(baseFile, fd, el, sl))
		{
			std::remove((baseFile).c_str());
			std::remove((baseFile + "_el").c_str());
			std::remove((baseFile + "_sl").c_str());
			CleanUpLockfile(el);
			CleanUpLockfile(sl);
			fcntl(fd, F_SETLEASE, F_UNLCK);
			close(fd);
		}
	}
}

static void CleanSemaphores()
{
	auto const filterSemaphores = [](auto const& dirent)
	{
		std::string const name = dirent.path().filename().string();
		// sem.fastdds_portXXXX_mutex
		// sem.fastrtps_portXXXX_mutex
		if(name.rfind("sem.fastdds_", 0) == std::string::npos && name.rfind("sem.fastrtps_", 0) == std::string::npos)
		{
			return false;
		}
		return true;
	};
	for(auto const& dirent :
	    std::filesystem::recursive_directory_iterator("/dev/shm") | std::views::filter(filterSemaphores))
	{
		std::string name = dirent.path().filename().string();
		name             = name.substr(std::strlen("sem."));
		sem_t* pSem      = sem_open(name.c_str(), O_EXCL, 0, 0);
		if(pSem != SEM_FAILED)
		{
			sem_unlink(name.c_str());
			sem_close(pSem);
		}
	}
}

namespace middleware
{
extern bool shmemCleanedUp;
void ShmCleanup(std::string const& dir = "/dev/shm")
{
	if(shmemCleanedUp)
	{
		return;
	}
	shmemCleanedUp = true;

	struct sigaction oldHandlerInfo{};
	sigaction(SIGIO, NULL, &oldHandlerInfo);
	// if the process isn't already handling SIGIO (which we might get due to fcntl F_SETLEASE), ignore it temporarily
	// (if we don't, receiving a SIGIO will kill the process)
	if(oldHandlerInfo.sa_handler == SIG_DFL)
	{
		struct sigaction ign{};
		ign.sa_handler = SIG_IGN;
		sigaction(SIGIO, &ign, NULL);
	}

	CleanPorts(dir);
	CleanSegments(dir);

	if(oldHandlerInfo.sa_handler == SIG_DFL)
	{
		struct sigaction dfl{};
		dfl.sa_handler = SIG_DFL;
		sigaction(SIGIO, &dfl, &oldHandlerInfo);
		// be paranoid, make sure we're not throwing away a handler that was registered in the ns since we first checked
		if(oldHandlerInfo.sa_handler != SIG_IGN)
		{
			sigaction(SIGIO, &oldHandlerInfo, NULL);
		}
	}

	// don't need SIGIO handled here, not acquiring any write locks
	CleanSemaphores();
}
}  // namespace middleware
