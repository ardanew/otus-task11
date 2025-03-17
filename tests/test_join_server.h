#pragma once
#include <gtest/gtest.h>
#include "server.h"

struct MocServer_Validate : public ::testing::Test, public Server
{
	MocServer_Validate() : Server(7777) {}
};