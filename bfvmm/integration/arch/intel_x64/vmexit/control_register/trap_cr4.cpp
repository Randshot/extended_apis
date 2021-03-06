//
// Bareflank Extended APIs
// Copyright (C) 2018 Assured Information Security, Inc.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include <bfvmm/vcpu/vcpu_factory.h>
#include <eapis/hve/arch/intel_x64/vcpu.h>

using namespace eapis::intel_x64;

// -----------------------------------------------------------------------------
// Handlers
// -----------------------------------------------------------------------------

uint64_t g_cr4;

bool
test_handler(
    gsl::not_null<vmcs_t *> vmcs, control_register_handler::info_t &info)
{
    bfignored(vmcs);
    bfignored(info);

    info.val = g_cr4;
    info.shadow = g_cr4;

    return false;
}

void
test_hlt_delegate(bfobject *obj)
{
    bfignored(obj);

    g_cr4 = ::intel_x64::cr4::get();
    ::intel_x64::cr4::set(0);

    if (::intel_x64::cr4::get() == g_cr4) {
        bfdebug_pass(0, "test");
    }
}

// -----------------------------------------------------------------------------
// vCPU
// -----------------------------------------------------------------------------

namespace test
{

class vcpu : public eapis::intel_x64::vcpu
{
public:
    explicit vcpu(vcpuid::type id) :
        eapis::intel_x64::vcpu{id}
    {
        this->add_hlt_delegate(
            hlt_delegate_t::create<test_hlt_delegate>()
        );

        eapis()->add_wrcr4_handler(
            0xFFFFFFFFFFFFFFFF,
            control_register_handler::handler_delegate_t::create<test_handler>()
        );

        eapis()->control_register()->enable_log();
    }
};

}

// -----------------------------------------------------------------------------
// vCPU Factory
// -----------------------------------------------------------------------------

namespace bfvmm
{

std::unique_ptr<vcpu>
vcpu_factory::make(vcpuid::type vcpuid, bfobject *obj)
{
    bfignored(obj);
    return std::make_unique<test::vcpu>(vcpuid);
}

}
