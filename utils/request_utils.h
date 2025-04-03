
#pragma once

namespace AsyncRequest {

enum Error
{
    OK,
    NoConnection,
    SendedAlready,
    WrongRequest,
    //NoResponse,
    // NoData,
    // ErrorData,
};
    
enum State {
    Idle,
    WaitWiFiConnection,
    RespondWaiting,
    SuccessRespond,
    WrongPayload,
    FailRespond,
};

};