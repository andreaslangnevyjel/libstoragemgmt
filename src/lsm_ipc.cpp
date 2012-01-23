/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: tasleson
 */

#include "lsm_ipc.hpp"

#include <libstoragemgmt/libstoragemgmt_plug_interface.h>

#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <list>

static std::string zeroPadNum(unsigned int num)
{
    std::ostringstream ss;
    ss << std::setw(Transport::HDR_LEN) << std::setfill('0') << num;
    return ss.str();
}

Transport::Transport() : s(-1)
{
}

Transport::Transport(int socket_desc) : s(socket_desc)
{
}

int Transport::sendMsg(const std::string &msg, int &error_code)
{
    int rc = -1;
    error_code = 0;

    if (msg.size() > 0) {
        ssize_t written = 0;
        //fprintf(stderr, ">>> %s\n", msg.c_str());
        std::string data = zeroPadNum(msg.size()) + msg;
        ssize_t msg_size = data.size();

        while (written < msg_size) {
            int wrote = send(s, data.c_str() + written, (msg_size - written),
                MSG_NOSIGNAL); //Prevent SIGPIPE on write
            if (wrote != -1) {
                written += wrote;
            } else {
                error_code = errno;
                break;
            }
        }

        if ((written == msg_size) && error_code == 0) {
            rc = 0;
        }
    }
    return rc;
}

static std::string readString(int fd, size_t count, int &error_code)
{
    char buff[4096];
    size_t amount_read = 0;
    std::string rc = "";

    error_code = 0;

    while (amount_read < count) {
        ssize_t rd = recv(fd, buff, std::min(sizeof(buff),
            (count - amount_read)),
            MSG_WAITALL);
        if (rd > 0) {
            amount_read += rd;
            rc += std::string(buff, rd);
        } else {
            error_code = errno;
            break;
        }
    }

    if ((amount_read == count) && (error_code == 0))
        return rc;
    else
        return std::string("");
}

std::string Transport::recvMsg(int &error_code)
{
    std::string msg;
    error_code = 0;
    std::string len = readString(s, HDR_LEN, error_code); //Read the length
    if (len.size() && error_code == 0) {
        msg = readString(s, strtoul(len.c_str(), NULL, 10), error_code);
        //fprintf(stderr, "<<< %s\n", msg.c_str());
    }
    return msg;
}

int Transport::getSocket(const std::string& path, int &error_code)
{
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    int rc = -1;
    error_code = 0;

    if (sfd != -1) {
        struct sockaddr_un addr;

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

        // Connect
        rc = connect(sfd, (struct sockaddr *) &addr, sizeof(addr));
        if (rc != 0) {
            error_code = errno;
            rc = -1; //Redundant, connect should set to -1 on error
        } else {
            rc = sfd; //We are good to go.
        }
    }
    return rc;
}

Transport::~Transport()
{
    close();
}

int Transport::close()
{
    int rc = EBADF;

    if (s >= 0) {
        int rc = ::close(s);
        if (rc != 0) {
            rc = errno;
        }

        // Regardless, clear out socket
        s = -1;
    }
    return rc;
}

ValueException::ValueException(std::string m) : std::runtime_error(m)
{
}

LsmException::LsmException(int code, std::string &msg) :
std::runtime_error(msg), error_code(code)
{

}

LsmException::~LsmException() throw ()
{
}

LsmException::LsmException(int code, std::string &msg,
    const std::string &debug_addl,
    const std::string &debug_data_addl) :
std::runtime_error(msg),
error_code(code), debug(debug_addl),
debug_data(debug_data_addl)
{
}

Value::Value(void) : t(null_t)
{
}

Value::Value(bool v) : t(boolean_t), s((v) ? "true" : "false")
{
}

Value::Value(double v) : t(numeric_t), s(to_string(v))
{
}

Value::Value(uint32_t v) : t(numeric_t), s(to_string(v))
{
}

Value::Value(int32_t v) : t(numeric_t), s(to_string(v))
{
}

Value::Value(uint64_t v) : t(numeric_t), s(to_string(v))
{
}

Value::Value(int64_t v) : t(numeric_t), s(to_string(v))
{
}

Value::Value(value_type type, const std::string &v) : t(type), s(v)
{
}

Value::Value(const std::vector<Value> &v) : t(array_t), array(v)
{
}

Value::Value(const char *v) : t(string_t), s(std::string(v))
{
}

Value::Value(const std::string &v) : t(string_t), s(v)
{
}

Value::Value(const std::map<std::string, Value> &v) : t(object_t), obj(v)
{
}

std::string Value::serialize(void)
{
    const unsigned char *buf;
    unsigned int len;
    std::string json;
    yajl_gen_config conf = {1, "  "};

    yajl_gen g = yajl_gen_alloc(&conf, NULL);

    if (g) {
        marshal(g);

        if (yajl_gen_status_ok == yajl_gen_get_buf(g, &buf, &len)) {
            json = std::string((const char*) buf);
        }
        yajl_gen_free(g);
    }
    return json;
}

Value::value_type Value::valueType()
{
    return t;
}

bool Value::hasKey(const std::string &k)
{
    if( t == object_t ) {
        std::map<std::string, Value>::iterator iter = obj.find(k);
        if (iter != obj.end() && iter->first == k) {
            return true;
        }
    }
    return false;
}

Value  Value::getValue( const char* key )
{
    if( hasKey(key) ) {
        return obj[key];
    }
    return Value();
}

void * Value::asVoid()
{
    if (t == null_t) {
        return NULL;
    }
    throw ValueException("Value not null");
}

bool Value::asBool()
{
    if (t == boolean_t) {
        return(s == "true");
    }
    throw ValueException("Value not boolean");
}

double Value::asDouble()
{
    if (t == numeric_t) {
        double rc;

        if (sscanf(s.c_str(), "%lf", &rc) > 0) {
            return rc;
        }
        throw ValueException("Value not a double");
    }
    throw ValueException("Value not numeric");
}

int32_t Value::asInt32_t()
{
    if (t == numeric_t) {
        int32_t rc;

        if (sscanf(s.c_str(), "%d", &rc) > 0) {
            return rc;
        }
        throw ValueException("Value not int32");
    }
    throw ValueException("Value not numeric");
}

int64_t Value::asInt64_t()
{
    if (t == numeric_t) {
        int64_t rc;
        if (sscanf(s.c_str(), "%lld", (long long int*) &rc) > 0) {
            return rc;
        }
        throw ValueException("Not an integer");
    }
    throw ValueException("Value not numeric");
}

uint32_t Value::asUint32_t()
{
    if (t == numeric_t) {
        uint32_t rc;
        if (sscanf(s.c_str(), "%u", &rc) > 0) {
            return rc;
        }
        throw ValueException("Not an integer");
    }
    throw ValueException("Value not numeric");
}

uint64_t Value::asUint64_t()
{
    if (t == numeric_t) {
        uint64_t rc;
        if (sscanf(s.c_str(), "%llu", (long long unsigned int*) &rc) > 0) {
            return rc;
        }
        throw ValueException("Not an integer");
    }
    throw ValueException("Value not numeric");
}

std::string Value::asString()
{
    if (t == string_t) {
        return s;
    }
    throw ValueException("Value not string");
}

std::map<std::string, Value> Value::asObject()
{
    if (t == object_t) {
        return obj;
    }
    throw ValueException("Value not object");
}

std::vector<Value> Value::asArray()
{
    if (t == array_t) {
        return array;
    }
    throw ValueException("Value not array");
}

void Value::marshal(yajl_gen g)
{
    switch (t) {
    case(null_t):
    {
        if (yajl_gen_status_ok != yajl_gen_null(g)) {
            throw ValueException("yajl_gen_null failure");
        }
        break;
    }
    case(boolean_t):
    {
        if (yajl_gen_status_ok != yajl_gen_bool(g, (s == "true") ? 1 : 0)) {
            throw ValueException("yajl_gen_bool failure");
        }
        break;
    }
    case(string_t):
    {
        if (yajl_gen_status_ok !=
            yajl_gen_string(g, (const unsigned char*) s.c_str(), s.size())) {
            throw ValueException("yajl_gen_string failure");
        }
        break;
    }
    case(numeric_t):
    {
        if (yajl_gen_status_ok != yajl_gen_number(g, s.c_str(), s.size())) {
            throw ValueException("yajl_gen_number failure");
        }
        break;
    }
    case(object_t):
    {

        if (yajl_gen_status_ok != yajl_gen_map_open(g)) {
            throw ValueException("yajl_gen_map_open failure");
        }

        std::map<std::string, Value>::iterator iter;

        for (iter = obj.begin(); iter != obj.end(); iter++) {
            if (yajl_gen_status_ok != yajl_gen_string(g,
                (const unsigned char*) iter->first.c_str(),
                iter->first.size())) {
                throw ValueException("yajl_gen_string failure");
            }
            iter->second.marshal(g);
        }

        if (yajl_gen_status_ok != yajl_gen_map_close(g)) {
            throw ValueException("yajl_gen_map_close failure");
        }
        break;
    }
    case(array_t):
    {
        if (yajl_gen_status_ok != yajl_gen_array_open(g)) {
            throw ValueException("yajl_gen_array_open failure");
        }

        for (unsigned int i = 0; i < array.size(); ++i) {
            array[i].marshal(g);
        }

        if (yajl_gen_status_ok != yajl_gen_array_close(g)) {
            throw ValueException("yajl_gen_array_close failure");
        }
        break;
    }
    }
}

class ParseElement {
public:

    enum parse_type {
        null, boolean, string, number, begin_map, end_map,
        begin_array, end_array, map_key, unknown
    };

    ParseElement() : t(unknown)
    {
    }

    ParseElement(parse_type type) : t(type)
    {
    }

    ParseElement(parse_type type, std::string value) : t(type), v(value)
    {
    }
    parse_type t;
    std::string v;

    std::string to_string()
    {
        return "type " + ::to_string(t) + ": value" + v;
    }
};

int handle_value(void * ctx, ParseElement::parse_type type)
{
    std::list<ParseElement> *l = (std::list<ParseElement> *)ctx;
    l->push_back(ParseElement(type));
    return 1;
}

int handle_value(void * ctx, ParseElement::parse_type type,
    const char* s, size_t len)
{
    std::list<ParseElement> *l = (std::list<ParseElement> *)ctx;
    l->push_back(ParseElement(type, std::string(s, len)));
    return 1;
}

static int handle_null(void * ctx)
{
    return handle_value(ctx, ParseElement::null);
}

static int handle_boolean(void * ctx, int boolean)
{
    std::string b = (boolean) ? "true" : "false";
    return handle_value(ctx, ParseElement::boolean, b.c_str(), b.size());
}

static int handle_number(void * ctx, const char *s, unsigned int len)
{
    return handle_value(ctx, ParseElement::number, s, len);
}

static int handle_string(void * ctx, const unsigned char * stringVal,
    unsigned int len)
{
    return handle_value(ctx, ParseElement::string, (const char*) stringVal, len);
}

static int handle_map_key(void * ctx, const unsigned char * stringVal,
    unsigned int len)
{
    return handle_value(ctx, ParseElement::map_key, (const char*) stringVal, len);
}

static int handle_start_map(void * ctx)
{
    return handle_value(ctx, ParseElement::begin_map);
}

static int handle_end_map(void * ctx)
{
    return handle_value(ctx, ParseElement::end_map);
}

static int handle_start_array(void * ctx)
{
    return handle_value(ctx, ParseElement::begin_array);
}

static int handle_end_array(void * ctx)
{
    return handle_value(ctx, ParseElement::end_array);
}

static yajl_callbacks callbacks = {
    handle_null,
    handle_boolean,
    NULL,
    NULL,
    handle_number,
    handle_string,
    handle_start_map,
    handle_map_key,
    handle_end_map,
    handle_start_array,
    handle_end_array
};

ParseElement get_next(std::list<ParseElement> &l)
{
    ParseElement rc = l.front();
    l.pop_front();
    return rc;
}

Value ParseElements(std::list<ParseElement> &l);

Value HandleArray(std::list<ParseElement> &l)
{
    std::vector<Value> values;

    ParseElement cur;

    if (!l.empty()) {
        do {
            cur = l.front();

            if (cur.t != ParseElement::end_array) {
                values.push_back(ParseElements(l));
            } else {
                get_next(l);
            }

        } while (!l.empty() && cur.t != ParseElement::end_array);
    }

    return Value(values);
}

Value HandleObject(std::list<ParseElement> &l)
{
    std::map<std::string, Value> values;
    ParseElement cur;

    if (!l.empty()) {
        do {
            cur = get_next(l);

            if (cur.t == ParseElement::map_key) {
                values[cur.v] = ParseElements(l);
            } else if (cur.t != ParseElement::end_map) {
                throw ValueException("Unexpected state: " + cur.to_string());
            }
        } while (!l.empty() && cur.t != ParseElement::end_map);
    }
    return Value(values);
}

Value ParseElements(std::list<ParseElement> &l)
{
    if (!l.empty()) {
        ParseElement cur = get_next(l);

        switch (cur.t) {
        case (ParseElement::null):
        case (ParseElement::boolean):
        case (ParseElement::string):
        case (ParseElement::number):
        {
            return Value((Value::value_type)cur.t, cur.v);
            break;
        }
        case (ParseElement::begin_map):
        {
            return HandleObject(l);
            break;
        }
        case (ParseElement::end_map):
        {
            throw ValueException("Unexpected end_map");
            break;
        }
        case (ParseElement::begin_array):
        {
            return HandleArray(l);
            break;
        }
        case( ParseElement::end_array):
        {
            throw ValueException("Unexpected end_array");
            break;
        }
        case( ParseElement::map_key):
        {
            throw ValueException("Unexpected map_key");
            break;
        }
        case( ParseElement::unknown):
        {
            throw ValueException("Unexpected unknown");
            break;
        }
        }
    }
    return Value();
}

std::string Payload::serialize(Value &v)
{
    return v.serialize();
}

Value Payload::deserialize(const std::string &json)
{
    yajl_handle hand;
    yajl_status stat;
    std::list<ParseElement> l;
    yajl_parser_config cfg = {1, 1};

    hand = yajl_alloc(&callbacks, &cfg, NULL, (void *) &l);

    if (hand) {
        stat = yajl_parse(hand, (const unsigned char*) json.c_str(), json.size());
        yajl_free(hand);

        if (stat == yajl_status_ok) {
            return ParseElements(l);
        } else {
            throw ValueException("In-valid json");
        }
    }
    return Value();
}

Ipc::Ipc()
{
}

Ipc::Ipc(int fd):t(fd)
{
}

Ipc::Ipc(std::string socket_path)
{
    int e = 0;
    int fd = Transport::getSocket(socket_path, e);
    if (fd >= 0) {
        t = Transport(fd);
    }
}

Ipc::~Ipc()
{
    t.close();
}

void Ipc::sendRequest(const std::string request, const Value &params, int32_t id)
{
    int rc = 0;
    int ec = 0;
    std::map<std::string, Value> v;

    v["method"] = Value(request);
    v["id"] = Value(id);
    v["params"] = params;

    Value req(v);
    rc = t.sendMsg(Payload::serialize(req), ec);

    if( rc != 0 ) {
        std::string em = std::string("Error sending message: errno ")
                            + ::to_string(ec);
        throw LsmException((int)LSM_ERROR_COMMUNICATION, em);
    }
}
Value Ipc::readRequest(void)
{
    int ec;
    std::string resp = t.recvMsg(ec);
    return Payload::deserialize(resp);
}

void Ipc::sendResponse(const Value &response, uint32_t id)
{
    int rc;
    int ec;
    std::map<std::string, Value> v;

    v["id"] = id;
    v["result"] = response;

    Value resp(v);
    rc = t.sendMsg(Payload::serialize(resp), ec);

    if( rc != 0 ) {
        std::string em = std::string("Error sending response: errno ")
                            + ::to_string(ec);
        throw LsmException((int)LSM_ERROR_COMMUNICATION, em);
    }
}

Value Ipc::readResponse()
{
    Value r = readRequest();
    if( r.hasKey(std::string("result"))) {
        return r.getValue("result");
    } else {
#warning "Add overloaded LsmException ctor which takes data argument"

        std::map<std::string, Value> rp = r.asObject();
        std::map<std::string, Value> error = rp["error"].asObject();

        std::string msg = error["message"].asString();
        throw LsmException((int)(error["code"].asInt32_t()), msg);
    }
}

Value Ipc::rpc(const std::string &request, const Value &params, int32_t id)
{
    sendRequest(request, params, id);
    return readResponse();
}