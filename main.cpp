#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <node_buffer.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <set>
#include <nan.h>

// midiprobe.cpp
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

int midiprobe()
{
  ::RtMidiIn  *midiin = 0;
  ::RtMidiOut *midiout = 0;
  // RtMidiIn constructor
  try {
    midiin = new ::RtMidiIn();
  }
  catch ( ::RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }
  // Check inputs.
  unsigned int nPorts = midiin->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
  std::string portName;
  for ( unsigned int i=0; i<nPorts; i++ ) {
    try {
      portName = midiin->getPortName(i);
    }
    catch ( ::RtMidiError &error ) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
  }
  // RtMidiOut constructor
  try {
    midiout = new ::RtMidiOut();
  }
  catch ( ::RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }
  // Check outputs.
  nPorts = midiout->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
  for ( unsigned int i=0; i<nPorts; i++ ) {
    try {
      portName = midiout->getPortName(i);
    }
    catch (::RtMidiError &error) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Output Port #" << i+1 << ": " << portName << '\n';
  }
  std::cout << '\n';
  // Clean up
 cleanup:
  delete midiin;
  delete midiout;
  return 0;
}

using namespace v8;
using namespace node;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_NUM(val) Nan::New<v8::Number>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)
#define JS_FN(val) Nan::New<v8::FunctionTemplate>(val)
#define JS_NEW(T, ...) Nan::New(T, __VA_ARGS__).ToLocalChecked()

namespace rtmidi_node {

  class RtMidiIn : public Nan::ObjectWrap {
    public:
      static NAN_MODULE_INIT(Initialize);
      virtual ~RtMidiIn();

    protected:
      RtMidiIn();
      static NAN_METHOD(New);
      static RtMidiIn *GetSelf(const Nan::FunctionCallbackInfo<Value>&);
      void Close();
      static NAN_METHOD(Close);

      static NAN_METHOD(GetPortCount);
      virtual unsigned int getPortCount() {
        return self_->getPortCount();
      }

      static NAN_METHOD(GetPortName);
      virtual std::string getPortName( unsigned int portNumber = 0 ) {
        return self_->getPortName(portNumber);
      }

      ::RtMidiIn* self_;
#if 0
  //! MIDI API specifier arguments.
  enum Api {
    UNSPECIFIED,    /*!< Search for a working compiled API. */
    MACOSX_CORE,    /*!< Macintosh OS-X Core Midi API. */
    LINUX_ALSA,     /*!< The Advanced Linux Sound Architecture API. */
    UNIX_JACK,      /*!< The JACK Low-Latency MIDI Server API. */
    WINDOWS_MM,     /*!< The Microsoft Multimedia MIDI API. */
    RTMIDI_DUMMY    /*!< A compilable but non-functional API. */
  };

  //! A static function to determine the current RtMidi version.
  static std::string getVersion( void ) throw();

  //! A static function to determine the available compiled MIDI APIs.
  /*!
    The values returned in the std::vector can be compared against
    the enumerated list values.  Note that there can be more than one
    API compiled for certain operating systems.
  */
  static void getCompiledApi( std::vector<RtMidi::Api> &apis ) throw();

  //! Pure virtual openPort() function.
  virtual void openPort( unsigned int portNumber = 0, const std::string &portName = std::string( "RtMidi" ) ) = 0;

  //! Pure virtual openVirtualPort() function.
  virtual void openVirtualPort( const std::string &portName = std::string( "RtMidi" ) ) = 0;

  //! Pure virtual getPortCount() function.
  virtual unsigned int getPortCount() = 0;

  //! Pure virtual getPortName() function.
  virtual std::string getPortName( unsigned int portNumber = 0 ) = 0;

  //! Pure virtual closePort() function.
  virtual void closePort( void ) = 0;

  //! Returns true if a port is open and false if not.
  /*!
      Note that this only applies to connections made with the openPort()
      function, not to virtual ports.
  */
  virtual bool isPortOpen( void ) const = 0;

  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is best
    to set the error callback function before opening a port.
  */
  virtual void setErrorCallback( RtMidiErrorCallback errorCallback = NULL, void *userData = 0 ) = 0;
#endif
  };


  /*
   * Helpers for dealing with RtMidi errors.
   */

  static inline const std::string&
  ErrorMessage(const ::RtMidiError &error) {
    return error.getMessage();
  }

  static inline Local<Value>
  ExceptionFromError(const ::RtMidiError &error) {
    return Nan::Error(ErrorMessage(error).c_str());
  }

  /*
   * RtMidiIn methods.
   */

  NAN_MODULE_INIT(RtMidiIn::Initialize) {
    Nan::HandleScope scope;
    Local<FunctionTemplate> t = JS_FN(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "close", Close);
    Nan::SetPrototypeMethod(t, "getPortCount", GetPortCount);
    Nan::SetPrototypeMethod(t, "getPortName", GetPortName);

    Nan::Set(target, JS_NEW("RtMidiIn"), Nan::GetFunction(t).ToLocalChecked());
  }


  RtMidiIn::~RtMidiIn() {
    Close();
  }

  NAN_METHOD(RtMidiIn::New) {
    assert(info.IsConstructCall());
    /*
    int arg = 1;
    if (info.Length() == 1) {
      if (!info[0]->IsNumber()) {
        return Nan::ThrowTypeError("arg must be an integer");
      }
      arg = Nan::To<int>(info[0]).FromJust();
      if (arg < 1) {
        return Nan::ThrowRangeError("arg must be a positive number");
      }
    }
    */
    RtMidiIn *context = new RtMidiIn();
    context->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  RtMidiIn::RtMidiIn() : Nan::ObjectWrap() {
    try {
      self_ = new ::RtMidiIn();
    }
    catch ( ::RtMidiError &error ) {
      if (!self_) Nan::ThrowError(ErrorMessage(error).c_str());
    }
  }

  RtMidiIn *
  RtMidiIn::GetSelf(const Nan::FunctionCallbackInfo<Value>& info) {
    return Nan::ObjectWrap::Unwrap<RtMidiIn>(info.This());
  }

  void
  RtMidiIn::Close() {
    if (self_ != NULL) {
      delete self_;
      self_ = NULL;
    }
  }

  NAN_METHOD(RtMidiIn::Close) {
    GetSelf(info)->Close();
    return;
  }

#if 0
  NAN_METHOD(RtMidiIn::SetOpt) {
    if (info.Length() != 2)
      return Nan::ThrowError("Must pass an option and a value");
    if (!info[0]->IsNumber() || !info[1]->IsNumber())
      return Nan::ThrowTypeError("Arguments must be numbers");
    int option = Nan::To<int32_t>(info[0]).FromJust();
    int value = Nan::To<int32_t>(info[1]).FromJust();

    RtMidiIn *context = GetRtMidiIn(info);
    /*
    if (zmq_ctx_set(context->self_, option, value) < 0)
      return Nan::ThrowError(ExceptionFromError());
    return;
    */
  }
#endif

  NAN_METHOD(RtMidiIn::GetPortCount) {
    RtMidiIn *context = GetSelf(info);
    int value = context->getPortCount();
    info.GetReturnValue().Set(JS_INT(value));
  }

  NAN_METHOD(RtMidiIn::GetPortName) {
    if (info.Length() != 1)
      return Nan::ThrowError("Must pass an index");
    if (!info[0]->IsNumber())
      return Nan::ThrowTypeError("Index must be an integer");
    int index = Nan::To<int32_t>(info[0]).FromJust();

    RtMidiIn *context = GetSelf(info);
    const std::string& value = context->getPortName(index);
    info.GetReturnValue().Set(JS_STR(value));
  }

  static NAN_MODULE_INIT(Initialize) {
    Nan::HandleScope scope;
    RtMidiIn::Initialize(target);
  }

}


// module

extern "C" NAN_MODULE_INIT(init) {
  rtmidi_node::Initialize(target);
}

NODE_MODULE(rtmidi, init)

