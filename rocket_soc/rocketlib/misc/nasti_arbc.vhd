-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @details   This file implements priority multiplexing of the slaves
--!            data to the upper connected Netowrok On Chip. 
--!            Slave with the index 0 has the the highest priority.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity NastiArbiter is
  port (
    clk  : in std_logic;
    i    : in  nasti_slaves_out_vector;
    o    : out nasti_slave_out_type
  );
end; 
 
architecture arch_NastiArbiter of NastiArbiter is
begin

  comblogic : process(i)
    variable t : nasti_slave_out_type;
  begin
 
    t := nasti_slave_out_none;

    for n in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
       t.ar_ready := t.ar_ready or i(n).ar_ready;
       t.aw_ready := t.aw_ready or i(n).aw_ready;
       t.w_ready  := t.w_ready or i(n).w_ready;

       if t.b_valid = '0' and i(n).b_valid = '1' then
          t.b_valid := '1';
          t.b_resp  := i(n).b_resp;
          t.b_id    := i(n).b_id;
          t.b_user  := i(n).b_user;
       end if;

       if t.r_valid = '0' and i(n).r_valid = '1' then
          t.r_valid := '1';
          t.r_resp  := i(n).r_resp;
          t.r_data  := i(n).r_data;
          t.r_last  := i(n).r_last;
          t.r_id    := i(n).r_id;
          t.r_user  := i(n).r_user;
       end if;
    end loop;
   
    o <= t;
  end process;

end;