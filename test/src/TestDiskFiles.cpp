// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <gtest/gtest.h>

#include <random>

#include <SimpleSysIO/SysCall/Files.hpp>


#ifdef SIMPLESYSIO_ENABLE_SYSCALL_FILESYSTEM


#ifndef SIMPLESYSIO_CUSTOMIZED_NAMESPACE
using namespace SimpleSysIO;
#else
using namespace SIMPLESYSIO_CUSTOMIZED_NAMESPACE;
#endif


namespace SimpleSysIO_Test
{
	extern size_t g_numOfTestFile;
}


GTEST_TEST(TestDiskFiles, CountTestFile)
{
	static auto tmp = ++SimpleSysIO_Test::g_numOfTestFile;
	(void)tmp;
}


static std::string GenRandomFileName()
{
	static constexpr const char sk_fileNameAlphabet[] =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789";
	static const size_t sk_randCharLen = 16;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 61);

	std::string fileName = "TestDiskFiles_";
	for (size_t i = 0; i < sk_randCharLen; ++i)
	{
		fileName += static_cast<char>(sk_fileNameAlphabet[dis(gen)]);
	}
	return fileName;
}


GTEST_TEST(TestDiskFiles, BinaryCreateWriteThenRead)
{
	std::string fileName = GenRandomFileName();

	std::string testingString = "Hello, world!";

	{
		// Open
		auto file = SysCall::WBinaryFile::Create(fileName);

		// Write testing string
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		auto fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size());

		// seek to current position
		file->Seek(0, SeekWhence::Current);

		// Write again
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 2);
	}

	{
		// Open
		auto file = SysCall::RBinaryFile::Open(fileName);

		std::string content;

		// Read all
		content = file->ReadBytes<std::string>();
		ASSERT_EQ(content, testingString + testingString);

		// Check file size
		auto fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 2);

		// seek to begin
		file->Seek(-1 * (testingString.size() * 2), SeekWhence::Current);

		// Read partial
		content = file->ReadBytes<std::string>(testingString.size());
		ASSERT_EQ(content, testingString);

		// Read till end but with larger count
		content = file->ReadBytes<std::string>(testingString.size() * 2);
		ASSERT_EQ(content, testingString);
	}

	// Clean up the testing file
	remove(fileName.c_str());
}


GTEST_TEST(TestDiskFiles, BinaryAppendWriteThenRead)
{
	std::string fileName = GenRandomFileName();

	std::string testingString = "Hello, world!";

	// Write something to append on
	{
		auto file = SysCall::WBinaryFile::Create(fileName);
		file->WriteBytes(testingString);
	}

	// Append
	{
		auto file = SysCall::WBinaryFile::Append(fileName);

		// Write testing string
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		auto fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 2);

		// seek to current position
		file->Seek(0, SeekWhence::Current);

		// Write again
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 3);
	}

	// Read to compare
	{
		auto file = SysCall::RBinaryFile::Open(fileName);

		std::string content;

		// Read all
		content = file->ReadBytes<std::string>();
		ASSERT_EQ(content, testingString + testingString + testingString);
	}

	// Clean up the testing file
	remove(fileName.c_str());
}


GTEST_TEST(TestDiskFiles, BinaryReadWriteCreate)
{
	std::string fileName = GenRandomFileName();

	std::string testingString = "Hello, world!";

	{
		// Open
		auto file = SysCall::RWBinaryFile::Create(fileName);

		// ===== Write =====

		// Write testing string
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		auto fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size());

		// seek to current position
		file->Seek(0, SeekWhence::Current);

		// Write again
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 2);

		// ===== Read =====

		// seek to begin
		file->Seek(0);

		std::string content;

		// Read all
		content = file->ReadBytes<std::string>();
		ASSERT_EQ(content, testingString + testingString);

		// seek to begin
		file->Seek(-1 * (testingString.size() * 2), SeekWhence::Current);

		// Read partial
		content = file->ReadBytes<std::string>(testingString.size());
		ASSERT_EQ(content, testingString);

		// Read till end but with larger count
		content = file->ReadBytes<std::string>(testingString.size() * 2);
		ASSERT_EQ(content, testingString);
	}

	// Clean up the testing file
	remove(fileName.c_str());
}


GTEST_TEST(TestDiskFiles, BinaryReadWriteAppend)
{
	std::string fileName = GenRandomFileName();

	std::string testingString = "Hello, world!";

	// Write something to append on
	{
		auto file = SysCall::WBinaryFile::Create(fileName);
		file->WriteBytes(testingString);
	}

	{
		// Open
		auto file = SysCall::RWBinaryFile::Append(fileName);

		// ===== Write =====

		// Write testing string
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		auto fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 2);

		// seek to current position
		file->Seek(0, SeekWhence::Current);

		// Write again
		file->WriteBytes(testingString);
		file->Flush();

		// Check file size
		fileSize = file->GetFileSize();
		ASSERT_EQ(fileSize, testingString.size() * 3);

		// ===== Read =====

		std::string content;

		// seek to begin
		file->Seek(0);

		// Read all
		content = file->ReadBytes<std::string>();
		ASSERT_EQ(content, testingString + testingString + testingString);
	}

	// Clean up the testing file
	remove(fileName.c_str());
}

#endif // SIMPLESYSIO_ENABLE_SYSCALL_FILESYSTEM
