require 'bindata'
require 'pry'
require 'socket'
require './libbluetooth'
require './ble'
require 'thread'

def ba2str(ba)
    sprintf "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", ba[5], ba[4], ba[3], ba[2], ba[1], ba[0]
end

@devices = {}
def put_advertising_devices(queue, so)
    loop do
        ss = StringIO.new
        ss << so.read(LibBluetooth::HCI_MAX_EVENT_SIZE)
        ss.rewind

        header = BLHeader.read(ss)

        if (header.subtype != 0x02)
            puts "t:#{header.subtype}"
            next
        end

        adv = BLEAdversingReport.read(ss)

        eir  = adv.parsed
        p eir
        addr = ba2str adv.bdaddr

        @devices[addr] ||= {}
        @devices[addr].merge!(eir)
        queue << addr
    end
end


adv_queue = Queue.new
scanner = Thread.new do
    was_sockopt = nil
    begin
        dev_id = LibBluetooth.hci_get_route(nil);
        throw "No hci device available" if dev_id < 0

        dd = LibBluetooth.hci_open_dev(dev_id);
        throw "cannot open hci device" if dd < 0

        own_type = 0x00
        scan_type = 0x01
        filter_type = 0
        filter_policy = 0x00
        interval = 0x0010
        window = 0x0010

        err = LibBluetooth.hci_le_set_scan_parameters(dd, scan_type, interval, window, own_type, filter_policy, 10000)
        throw :hci_le_set_scan_parameters if err < 0

        err = LibBluetooth.hci_le_set_scan_enable(dd, 0x01, 0x00, 10000);
        throw :hci_le_set_scan_enable if err < 0

        so = Socket.for_fd(dd)
        was_sockopt = so.getsockopt(LibBluetooth::SOL_HCI, LibBluetooth::HCI_FILTER).to_s
        hci = HciFilter.new
        hci.hci_event_pkt = 1
        hci.evt_le_meta_event = 1
        p so.setsockopt(LibBluetooth::SOL_HCI, LibBluetooth::HCI_FILTER, hci.to_binary_s)

        put_advertising_devices(adv_queue, so);

    ensure
        if !dd.nil?
            LibBluetooth.hci_le_set_scan_enable(dd, 0x00, 0x00, 10000);
            if !was_sockopt.nil?
                puts "final"
                so = Socket.for_fd(dd)
                p so.setsockopt(LibBluetooth::SOL_HCI, LibBluetooth::HCI_FILTER, was_sockopt)
                so.close
            end
            LibBluetooth.hci_close_dev(dd);
        end
    end
end



def gatt_discover_primary
end



printer = Thread.new do
    loop do
        addr = adv_queue.pop
        eir  = @devices[addr]
        next unless eir.has_key?(:BT_EIR_UUID128_ALL) && eir[:BT_EIR_UUID128_ALL].include?("9ecadc240ee5a9e093f3a3b50100406e")
#        next unless eir.has_key?(:BT_EIR_UUID128_ALL) && eir[:BT_EIR_UUID128_ALL].include?("babad1d10ee5a9e093f3a3b50100406e")

        puts "found tty #{addr} -< #{@devices[addr][:BT_EIR_NAME_COMPLETE]}"
        gatt_discover_primary

    end
end

Thread.abort_on_exception = true
scanner.join
printer.join
