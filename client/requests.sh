#!/usr/bin/env bash

HOST="${1:-127.0.0.1}"
PORT="${2:-8080}"
BASE_URL="http://${HOST}:${PORT}"

GREEN='\033[1;32m'
RED='\033[1;31m'
NC='\033[0m'

send_request(){
  local test_name="$1"
  local endpoint="$2"
  local method="${3:-GET}"
  
  echo -e "\n========================================"
  echo -e "TEST: ${test_name}"
  echo -e "HTTP ${method} ${BASE_URL}${endpoint}"
  echo -e "========================================\n"
  
  local raw_response=$(curl -i -s -X "${method}" "${BASE_URL}${endpoint}")
  local http_status
  http_status=$(echo -e "${raw_response}" | head -n 1 | tr -d '\r')
  local status_color="${RED}"
  if [[ "${http_status}" == *"200 OK"* ]]; then
    status_color="${GREEN}"
  fi
  echo -e "Status: ${status_color}${http_status}${NC}"

  local json_body=$(echo -e "${raw_response}" | sed '1,/^\r\{0,1\}$/d')
  echo -e "\nResponse Body:"
  if command -v jq &> /dev/null; then
    echo -e "${json_body}" | jq .
  else
    echo -e "${json_body}"
  fi

  echo -e "\n"
}

send_request "1. Выборка по since и level (HTTP 200 OK)" "/journal?since=2020-01-01&level=2"
send_request "2. Выборка только по since, level=0 по умолчанию (HTTP 200 OK)" "/journal?since=2020-01-01"
send_request "3. Выборка с точным timestamp (HTTP 200 OK)" "/journal?since=2025-08-22%2014:11:13%2B00&level=3"
send_request "4. Выборка с учетом смещения +03 (HTTP 200 OK)" "/journal?since=2025-08-22%2017:11:13%2B03&level=3"
send_request "5. Отсутствует обязательный параметр since (HTTP 400 Bad Request)" "/journal?level=3"
send_request "6. Некорректный нечисловой параметр level (HTTP 400 Bad Request)" "/journal?since=2020-01-01&level=abc"
send_request "7. Запрос к неизвестному пути (HTTP 404 Not Found)" "/unknown"
send_request "8. Неподдерживаемый HTTP метод POST (HTTP 400 Bad Request)" "/journal?since=2025-08-22%2014:11:13%2B00&level=3" "POST"


