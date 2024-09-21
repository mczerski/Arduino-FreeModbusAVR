#!/usr/bin/env python
import sys
import math
import argparse
import pymodbus.client as ModbusClient
from pymodbus.file_message import FileRecord


def read_image(image_file):
    with open(image_file, 'rb') as f:
        whole_file = f.read()
    if len(whole_file) % 2 == 1:
        whole_file += b'\0'
    return whole_file


def write_file(whole_file, file_number, slave):
    record_number = 0
    while 2 * record_number < len(whole_file):
        record_length = min(64, (len(whole_file) - 2 * record_number) // 2)
        data = whole_file[2 * record_number:2 * record_number + 128]
        file_record = FileRecord(
            record_data=data,
            reference_type=6,
            file_number=file_number,
            record_number=record_number,
            record_length=record_length,
        )
        response = client.write_file_record([file_record], slave=slave)
        progress = int(100 * 2 * (record_number + record_length + 1) / len(whole_file))
        print(f'{progress: >3}%', end='\r')
        if response.isError():
            print(f'\nError while writing on record {record_number}. {response}')
            sys.exit(1)
        record_number += record_length


def verify_file(whole_file, file_number, slave):
    record_number = 0
    while 2 * record_number < len(whole_file):
        record_length = min(64, (len(whole_file) - 2 * record_number) // 2)
        data = whole_file[2 * record_number:2 * record_number + 128]
        file_record = FileRecord(
            reference_type=6,
            file_number=file_number,
            record_number=record_number,
            record_length=record_length,
        )
        response = client.read_file_record([file_record], slave=slave)
        progress = int(100 * 2 * (record_number + record_length + 1) / len(whole_file))
        print(f'{progress: >3}%', end='\r')
        if response.isError():
            print(f'\nError while reading on record {record_number}. {response}')
            sys.exit(1)
        if data != response.records[0].record_data:
            print(f'\nData mismatch for record {record_number}\nExpected: {data.hex()}\nActual: {response.records[0].record_data.hex()}')
            sys.exit(1)
        record_number += record_length


def read_file(file_size, file_number, slave):
    if file_size % 2 == 1:
        file_size += 1
    record_number = 0
    whole_file = b''
    while 2 * record_number < file_size:
        record_length = min(64, (file_size - 2 * record_number) // 2)
        file_record = FileRecord(
            reference_type=6,
            file_number=file_number,
            record_number=record_number,
            record_length=record_length,
        )
        response = client.read_file_record([file_record], slave=slave)
        if response.isError():
            print(f'Error while reading on record {record_number}. {response}')
            sys.exit(1)
        whole_file += response.records[0].record_data
        record_number += record_length
    return whole_file


def make_image_header(whole_file):
    image_size = len(whole_file)
    header = b'FLXIMG:' + image_size.to_bytes(2, 'big') + b':'
    return header

parser = argparse.ArgumentParser(prog='Modbus file uploader')
parser.add_argument('-d', '--device', help='Path to serial device', required=True)
parser.add_argument('-b', '--baud', help='Baudrate. Default is 9600', required=False, default=9600, type=int)
parser.add_argument('-f', '--file', help='File to upload', required=False)
parser.add_argument('-s', '--slave', help='Modbus slave id', type=int, default=1)
parser.add_argument('-x', '--dump-size', help='Dump firmware instead of image write', type=int, default=0)
parser.add_argument('-v', '--version', help='Dump firmware version instead of image write', action='store_true')

args = parser.parse_args()

client = ModbusClient.ModbusSerialClient(
    args.device,
    baudrate=args.baud,
    bytesize=8,
    parity="N",
    stopbits=1,
)
client.connect()

if args.dump_size > 0:
    whole_file = read_file(args.dump_size, 1, slave=args.slave)
    with open(args.file, 'wb') as f:
        f.write(whole_file)
    header = read_file(10, 2, slave=args.slave)
    print(header)
    sys.exit(0)

if args.version:
    response = client.report_slave_id(slave=args.slave)
    slave_id = str(int(response.identifier[0]))
    version = response.identifier[2:].decode()
    print('ID: ' + slave_id)
    print('Version: ' + version)
    sys.exit(0)

whole_file = read_image(args.file)

print('Writing image file')
write_file(whole_file, file_number=1, slave=args.slave)
print('Veryfing image file')
verify_file(whole_file, file_number=1, slave=args.slave)

image_header = make_image_header(whole_file)
print('Writing image header')
write_file(image_header, file_number=2, slave=args.slave)
#print('Verifying image header')
#verify_file(image_header, file_number=2)
print('Success')
