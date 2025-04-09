
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

namespace OpenWeaterRequest {
    const char uri[] PROGMEM = "https://api.openweathermap.org/data/2.5/weather?";
};