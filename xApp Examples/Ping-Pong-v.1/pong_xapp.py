import json,os
from ricxappframe.xapp_frame import RMRXapp, rmr

os.environ['DBAAS_SERVICE_PORT'] = '6379'
os.environ['DBAAS_SERVICE_HOST'] = '127.0.0.1'
os.environ['RMR_SEED_RT'] = 'test_route_test.rt'
os.environ['RMR_LOG_VLEVEL'] = '0'
os.environ['RMR_RTG_SVC']='-1'


def post_init(_self):
    """post init"""
    print("pong xapp could do some useful stuff here!")


def sixtyh(self, summary, sbuf):
    self.logger.info("pong registered 1234 handler called!")
    # see comment in ping about this; bytes does not work with the ric mdc logger currently
    print("pong 1234 handler received: {0}".format(summary))
    jpay = json.loads(summary['payload'])
    print("Time",jpay['ping'],"\n")
    self.rmr_rts(sbuf, new_payload=json.dumps({"ACK": "I got your message"}).encode(), new_mtype=6000, retries=100)
    self.rmr_free(sbuf)

def test(self, summary, sbuf):
    self.logger.info("pong registered 7777 handler called!")
    # see comment in ping about this; bytes does not work with the ric mdc logger currently
    print("7777")
    jpay = json.loads(summary['payload'])
    print("Time",jpay['ping'],"\n")
    self.rmr_rts(sbuf, new_payload=json.dumps({"ACK": "I got your message"}).encode(), new_mtype=6000, retries=100)
    self.rmr_free(sbuf)


def defh(self, summary, sbuf):
    """default callback"""
    self.logger.info("pong default handler called!")
    print("pong default handler received: {0}".format(summary))
    self.rmr_free(sbuf)


xapp = RMRXapp(default_handler=defh, rmr_port=4560, post_init=post_init, use_fake_sdl=True)
xapp.register_callback(sixtyh, 1234)
xapp.register_callback(test, 7777)
xapp.run()  # will not thread by default
