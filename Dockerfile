# multi-stage build
FROM alpine:3.11.6 AS os
RUN set -x \
  && apk add --update --no-cache \
    jq \
    libreoffice \
    qpdf-libs \
    ttf-freefont \
    msttcorefonts-installer \
    tini \
  && update-ms-fonts \
  && fc-cache -f
WORKDIR /app


FROM os AS build
RUN set -x \
  && apk add --update --no-cache \
    libreofficekit \
    libreoffice-sdk \
    qpdf-dev \
    g++ \
    make

COPY main/ /app/main/
RUN sh -c "cd /app/main && make -j$(nproc)"


FROM overview/overview-convert-framework:0.0.17 AS framework

FROM os AS base
WORKDIR /app
COPY --from=framework /app/run /app/run
COPY --from=framework /app/convert-single-file /app/convert
ENV OFFICE_PATH=/usr/lib/libreoffice
ENV LD_LIBRARY_PATH=/usr/lib/libreoffice/program
COPY ./do-convert-single-file /app/do-convert-single-file
COPY --from=build /app/main/convert-to-pdf-with-metadata /app/convert-to-pdf-with-metadata
CMD [ "tini", "--", "/app/run" ]


FROM base AS test
COPY --from=framework /app/test-convert-single-file /app/
COPY test/ /app/test/
ENV TIMEOUT 5
RUN [ "/app/test-convert-single-file" ]
CMD [ "true" ]


FROM base AS production
