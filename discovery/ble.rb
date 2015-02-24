require 'bindata'

class BLHeader < BinData::Record
    endian :little
    uint8  :dunno1
    uint8  :dunno2
    uint8  :probably_payload_size
    uint8  :subtype
end

class BLEAdversingReport < BinData::Record
    uint8  :garbage
    uint8  :garbage2
    uint8  :bdaddr_type
    array  :bdaddr, :type => :uint8, :initial_length => 6
    uint8  :data_len
    string :data, :read_length => lambda { data_len }

    BT_EIR_FLAGS          = 0x01
    BT_EIR_UUID16_SOME    = 0x02
    BT_EIR_UUID16_ALL     = 0x03
    BT_EIR_UUID32_SOME    = 0x04
    BT_EIR_UUID32_ALL     = 0x05
    BT_EIR_UUID128_SOME   = 0x06
    BT_EIR_UUID128_ALL    = 0x07
    BT_EIR_NAME_SHORT     = 0x08
    BT_EIR_NAME_COMPLETE  = 0x09
    BT_EIR_TX_POWER       = 0x0a
    BT_EIR_CLASS_OF_DEV   = 0x0d
    BT_EIR_SSP_HASH_P192  = 0x0e
    BT_EIR_SSP_RANDOMIZER_P192  = 0x0f
    BT_EIR_DEVICE_ID      = 0x10
    BT_EIR_SMP_TK         = 0x10
    BT_EIR_SMP_OOB_FLAGS  = 0x11
    BT_EIR_SLAVE_CONN_INTERVAL  = 0x12
    BT_EIR_SERVICE_UUID16       = 0x14
    BT_EIR_SERVICE_UUID128      = 0x15
    BT_EIR_SERVICE_DATA   = 0x16
    BT_EIR_PUBLIC_ADDRESS = 0x17
    BT_EIR_RANDOM_ADDRESS = 0x18
    BT_EIR_GAP_APPEARANCE = 0x19
    BT_EIR_ADVERTISING_INTERVAL = 0x1a
    BT_EIR_LE_DEVICE_ADDRESS    = 0x1b
    BT_EIR_LE_ROLE        = 0x1c
    BT_EIR_SSP_HASH_P256  = 0x1d
    BT_EIR_SSP_RANDOMIZER_P256  = 0x1e
    BT_EIR_3D_INFO_DATA   = 0x3d
    BT_EIR_MANUFACTURER_DATA    = 0xff

    def self.eir_flag_str(eir)
        case eir
        when BT_EIR_FLAGS
            :BT_EIR_FLAGS
        when BT_EIR_UUID16_SOME
            :BT_EIR_UUID16_SOME
        when BT_EIR_UUID16_ALL
            :BT_EIR_UUID16_ALL
        when BT_EIR_UUID32_SOME
            :BT_EIR_UUID32_SOME
        when BT_EIR_UUID32_ALL
            :BT_EIR_UUID32_ALL
        when BT_EIR_UUID128_SOME
            :BT_EIR_UUID128_SOME
        when BT_EIR_UUID128_ALL
            :BT_EIR_UUID128_ALL
        when BT_EIR_NAME_SHORT
            :BT_EIR_NAME_SHORT
        when BT_EIR_NAME_COMPLETE
            :BT_EIR_NAME_COMPLETE
        when BT_EIR_TX_POWER
            :BT_EIR_TX_POWER
        when BT_EIR_CLASS_OF_DEV
            :BT_EIR_CLASS_OF_DEV
        when BT_EIR_SSP_HASH_P192
            :BT_EIR_SSP_HASH_P192
        when BT_EIR_SSP_RANDOMIZER_P192
            :BT_EIR_SSP_RANDOMIZER_P192
        when BT_EIR_DEVICE_ID
            :BT_EIR_DEVICE_ID
        when BT_EIR_SMP_TK
            :BT_EIR_SMP_TK
        when BT_EIR_SMP_OOB_FLAGS
            :BT_EIR_SMP_OOB_FLAGS
        when BT_EIR_SLAVE_CONN_INTERVAL
            :BT_EIR_SLAVE_CONN_INTERVAL
        when BT_EIR_SERVICE_UUID16
            :BT_EIR_SERVICE_UUID16
        when BT_EIR_SERVICE_UUID128
            :BT_EIR_SERVICE_UUID128
        when BT_EIR_SERVICE_DATA
            :BT_EIR_SERVICE_DATA
        when BT_EIR_PUBLIC_ADDRESS
            :BT_EIR_PUBLIC_ADDRESS
        when BT_EIR_RANDOM_ADDRESS
            :BT_EIR_RANDOM_ADDRESS
        when BT_EIR_GAP_APPEARANCE
            :BT_EIR_GAP_APPEARANCE
        when BT_EIR_ADVERTISING_INTERVAL
            :BT_EIR_ADVERTISING_INTERVAL
        when BT_EIR_LE_DEVICE_ADDRESS
            :BT_EIR_LE_DEVICE_ADDRESS
        when BT_EIR_LE_ROLE
            :BT_EIR_LE_ROLE
        when BT_EIR_SSP_HASH_P256
            :BT_EIR_SSP_HASH_P256
        when BT_EIR_SSP_RANDOMIZER_P256
            :BT_EIR_SSP_RANDOMIZER_P256
        when BT_EIR_3D_INFO_DATA
            :BT_EIR_3D_INFO_DATA
        when BT_EIR_MANUFACTURER_DATA
            :BT_EIR_MANUFACTURER_DATA
        else
            eir
        end
    end

    def parsed
        data = self.data.unpack("C*")
        offset = 0
        datah = {}
        while (offset < data.size)
            field_len  = data[offset]
            field_type = data[offset + 1]
            field_data = data[offset + 2, field_len - 1]

            break if (field_len == 0)
            break if (offset + field_len > data.size)

            datah[BLEAdversingReport.eir_flag_str field_type] = parse_eir_part(field_type, field_data)

            offset += field_len + 1;
        end
        datah
    end

    private
    def parse_eir_part(field_type, field_data)
        case field_type
        when BT_EIR_NAME_SHORT
            field_data.pack("C*")
        when BT_EIR_NAME_COMPLETE
            field_data.pack("C*")
        when BT_EIR_GAP_APPEARANCE
            field_data.pack("C*").unpack("v")[0]
        when BT_EIR_UUID16_ALL , BT_EIR_UUID16_SOME
            uuid16 = []
            field_data.pack('C*').unpack('n*').each do |u|
                uuid16 <<  "0x#{u.to_s(16)}"
            end
            uuid16
        when BT_EIR_UUID128_ALL, BT_EIR_UUID128_SOME
            uuid128 = []
            field_data.each_slice(16) do |u|
                uuid128 << u.map{|x| sprintf '%.2x', x}.join
            end
            uuid128
        else
            field_data
        end
    end


end
