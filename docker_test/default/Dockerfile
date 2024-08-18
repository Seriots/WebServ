FROM alpine AS base

RUN apk update
RUN apk add --no-cache curl python3 py3-requests

FROM base AS outer
COPY out_script.py /tmp/test.py

ENTRYPOINT ["python3", "/tmp/test.py"]

FROM base AS iner
COPY in_script.py /tmp/test.py

ENTRYPOINT ["python3", "/tmp/test.py"]
