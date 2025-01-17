/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/os/Network.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/WrapperSingle.h>
#include <yarp/dev/tests/IPositionControlTest.h>
#include <yarp/dev/tests/ITorqueControlTest.h>
#include <yarp/dev/tests/IAxisInfoTest.h>

#include <catch2/catch_amalgamated.hpp>
#include <harness.h>

using namespace yarp::dev;
using namespace yarp::os;

TEST_CASE("dev::RemoteControlBoardTest", "[yarp::dev]")
{
    YARP_REQUIRE_PLUGIN("fakeMotionControl", "device");
    YARP_REQUIRE_PLUGIN("controlBoard_nws_yarp", "device");
    YARP_REQUIRE_PLUGIN("remote_controlboard", "device");

    Network::setLocalMode(true);

    SECTION("Checking remote_controlboard device")
    {
        PolyDriver ddmc;
        PolyDriver ddnws;
        PolyDriver ddnwc;

        IPositionControl* ipos = nullptr;
        ITorqueControl* itrq = nullptr;
        IAxisInfo* iinfo = nullptr;

        ////////"Checking opening map2DServer and map2DClient polydrivers"
        {
            Property p_cfg;
            p_cfg.put("device", "fakeMotionControl");
            Property& grp = p_cfg.addGroup("GENERAL");
            grp.put("Joints", 2);
            REQUIRE(ddmc.open(p_cfg));
        }
        yarp::os::Time::delay(0.1);
        {
            Property p_cfg;
            p_cfg.put("device", "controlBoard_nws_yarp");
            p_cfg.put("name", "/controlboardserver");
            REQUIRE(ddnws.open(p_cfg));
        }
        {
            yarp::dev::WrapperSingle* ww_nws; ddnws.view(ww_nws);
            bool result_att = ww_nws->attach(&ddmc);
            REQUIRE(result_att);
        }
        yarp::os::Time::delay(0.1);
        {
            Property p_cfg;
            p_cfg.put("device", "remote_controlboard");
            p_cfg.put("local", "/local_controlboard");
            p_cfg.put("remote", "/controlboardserver");
            REQUIRE(ddnwc.open(p_cfg));
        }

        ddnwc.view(ipos);
        ddnwc.view(itrq);
        ddnwc.view(iinfo);
        yarp::dev::tests::exec_iPositionControl_test_1(ipos);
        yarp::dev::tests::exec_iTorqueControl_test_1(itrq);
        yarp::dev::tests::exec_iAxisInfo_test_1(iinfo);

        //"Close all polydrivers and check"
        {
            CHECK(ddnwc.close());
            CHECK(ddnws.close());
            CHECK(ddmc.close());
        }
    }

    Network::setLocalMode(false);
}
