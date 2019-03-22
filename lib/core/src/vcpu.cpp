#include "..\include\sleepy\vcpu.hpp"
#include <sleepy/vcpu.hpp>

#include <iostream>
#include <iterator>

#include <sleepy/memory.hpp>

namespace sleepy
{
    vcpu::vcpu(sleepy::memory& mem)
        : _mem(mem)
        , _vfw(vcpu_impl(this, &_mem, &_regs))
    { 
        _pre_exec_debug_fun  = [](vcpu&, const vcpu_instruction*) { return; };
        _post_exec_debug_fun = [](vcpu&, const vcpu_instruction*) { return; };
    }

    void vcpu::exec_op(opcode op, u8* args)
    {
        auto& inst = _vfw.inst_map[op];

        if(inst.cycles == 0xFF)
        {
            throw std::logic_error("Unimplemented opcode (dec): " + inst.op.value);
        }

        _pre_exec_debug_fun(*this, &inst);
        inst.call(args);
        _post_exec_debug_fun(*this, &inst);
        
        _ticks_elapsed += inst.cycles;
        _last_executed_inst = &inst;
        _regs.pc += inst.pc_offset;
    }

    const memory& vcpu::memory() const noexcept
    {
        return _mem;
    }

    const registers& vcpu::registers() const noexcept
    {
        return _regs;
    }

    const vcpu_instruction& vcpu::get_inst_data(opcode op) const
    {
        return _vfw.inst_map[op];
    }

    void vcpu::exec_next_tick()
    {
        u8 op = _mem.read_byte(_regs.pc);
        if (op == 0xCBu)
        {
            ++(_regs.pc);
            u8 opv = _mem.read_byte(_regs.pc);
            opcode opc(op, opv);
            u8* args = &_mem.data()[_regs.pc + 1];
            exec_op(opc, args);
        }
        else
        {
            opcode opc(op);
            u8* args = &_mem.data()[_regs.pc + 1];
            exec_op(opc, args);
        }
    }

    void vcpu::setup_memory(std::istream& data)
    {
        std::istreambuf_iterator<char> beg(data);
        std::istreambuf_iterator<char> end;

        std::vector<u8> cdata(beg, end);

        std::copy(cdata.begin(), cdata.end(), _mem.data());
        _memory_set = true;
    }

    void vcpu::setup_debug(debug_func_t pre_exec, debug_func_t post_exec)
    {
        _pre_exec_debug_fun = pre_exec;
        _post_exec_debug_fun = post_exec;
        _debug_enabled = true;
    }

    void vcpu::enable_debug(bool enabled)
    {
        _debug_enabled = enabled;
    }

    const vcpu_instruction* vcpu::last_executed_instruction() const noexcept
    {
        return _last_executed_inst;
    }

    void vcpu::delay_cycles(size_t count) noexcept
    {
        _ticks_elapsed += count;
    }

    uint64_t vcpu::elapsed_cycles() const noexcept
    {
        return _ticks_elapsed;
    }

    void vcpu::init_state()
    {
        _regs.a = 0x01;
        _regs.f = 0xB0;
        _regs.bc(0x0013);
        _regs.de(0x00D8);
        _regs.hl(0x014D);
        _regs.sp = 0xFFFE;

        _mem.write_byte(0xFF05, 0x00);
        _mem.write_byte(0xFF06, 0x00);
        _mem.write_byte(0xFF07, 0x00);
        _mem.write_byte(0xFF10, 0x80);
        _mem.write_byte(0xFF11, 0xBF);
        _mem.write_byte(0xFF12, 0xF3);
        _mem.write_byte(0xFF14, 0xBF);
        _mem.write_byte(0xFF16, 0x3F);
        _mem.write_byte(0xFF17, 0x00);
        _mem.write_byte(0xFF19, 0xBF);
        _mem.write_byte(0xFF1A, 0x7F);
        _mem.write_byte(0xFF1B, 0xFF);
        _mem.write_byte(0xFF1C, 0x9F);
        _mem.write_byte(0xFF1E, 0xBF);
        _mem.write_byte(0xFF20, 0xFF);
        _mem.write_byte(0xFF21, 0x00);
        _mem.write_byte(0xFF22, 0x00);
        _mem.write_byte(0xFF23, 0xBF);
        _mem.write_byte(0xFF24, 0x77);
        _mem.write_byte(0xFF25, 0xF3);
        _mem.write_byte(0xFF26, 0xF1);
        _mem.write_byte(0xFF40, 0x91);
        _mem.write_byte(0xFF42, 0x00);
        _mem.write_byte(0xFF43, 0x00);
        _mem.write_byte(0xFF45, 0x00);
        _mem.write_byte(0xFF47, 0xFC);
        _mem.write_byte(0xFF48, 0xFF);
        _mem.write_byte(0xFF49, 0xFF);
        _mem.write_byte(0xFF4A, 0x00);
        _mem.write_byte(0xFF4B, 0x00);
        _mem.write_byte(0xFFFF, 0x00);
    }
}