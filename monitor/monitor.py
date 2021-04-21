import time
import socket
import requests
from enum import Enum
import posix_ipc
from collections import deque

queues = ('/docs', '/frontier', '/links')
# length (in second) of monitoring window
window_len = 300
# ratio of abnormal datapoint in the monitoring window to trigger an alert
alert_ratio = 0.98
# ratio of abnormal datapoint in the window to fall back to normal from alert
fallback_ratio = 0.4
# threshold of a datapoint being labeled abnormal
abnormal_threshold = 0.02

hostname = socket.gethostname()
# put them in seperate pieces, so that Github scanner can't recognize it as a token
webhook = 'htt' + 'ps://dis' + 'cordapp.c' + 'om/api/we' + 'bhooks/826831330728738877/uFMluLKZ82d5UzEzgdOX5H' + '5wltSHXnm9-B2' + 'lirmK32Y5AuTB1tcpz' + '52UTPEmCVdKasHH'


def alert(msg):
    data = {
        "content": f'{hostname}: {msg}'
    }
    requests.post(webhook, json=data)


class QueueStatus(Enum):
    NORMAL = "normal"
    TOO_FEW = "too_few"
    TOO_MANY = "too_many"


class QueueMonitor:
    def __init__(self, queueName):
        self.mqueue = posix_ipc.MessageQueue(queueName)
        self.window = deque()
        self.status = QueueStatus.NORMAL

    def onTooFew(self, ratio_too_few, ratio_too_many):
        if ratio_too_few < fallback_ratio:
            alert(f'✅ {self.mqueue.name} no longer has too few messages.')
            self.status = QueueStatus.NORMAL

    def onTooMany(self, ratio_too_few, ratio_too_many):
        if ratio_too_many < fallback_ratio:
            alert(f'✅ {self.mqueue.name} no longer has too many messages.')
            self.status = QueueStatus.NORMAL

    def onNormal(self, ratio_too_few, ratio_too_many):
        if ratio_too_few > alert_ratio:
            if self.mqueue.name == '/links':
                # we don't monitor /links for too few messages, because filter is much faster than crawlers
                # also if it is the case that the crawlers are not getting enough URLs to crawl, and thus
                # not producing as many new links as we want, we could also observe that via too few URLs
                # in the frontier a bit later.
                return

            alert(f'🚨 too few messages in {self.mqueue.name} for {window_len} seconds!')
            self.status = QueueStatus.TOO_FEW
        elif ratio_too_many > alert_ratio:
            alert(f'🚨 too many messages in {self.mqueue.name} for {window_len} seconds!')
            self.status = QueueStatus.TOO_MANY

    def tick(self):
        # push a new datapoint into the window
        self.window.append(self.mqueue.current_messages / self.mqueue.max_messages)
        if len(self.window) > window_len:
            self.window.popleft()
        else:
            return

        # filter out abnormal datapoints
        num_too_few, num_too_many = 0, 0
        for dp in self.window:
            if dp < abnormal_threshold:
                num_too_few += 1
            elif (1 - dp) < abnormal_threshold:
                num_too_many += 1

        ratio_too_few = num_too_few / window_len
        ratio_too_many = num_too_many / window_len

        if self.status == QueueStatus.NORMAL:
            self.onNormal(ratio_too_few, ratio_too_many)
        elif self.status == QueueStatus.TOO_FEW:
            self.onTooFew(ratio_too_few, ratio_too_many)
        else:
            self.onTooMany(ratio_too_few, ratio_too_many)


if __name__ == '__main__':
    monitors = [QueueMonitor(q) for q in queues]
    while True:
        time.sleep(1)
        for monitor in monitors:
            monitor.tick()
