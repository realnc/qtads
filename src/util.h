// This is copyrighted software. More information is at the end of this file.
#pragma once
#include <QApplication>
#include <QtGlobal>
#include <type_traits>

/* Some missing stuff from >=Qt 5.7 for use when building with older Qt versions.
 *
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2016 Intel Corporation.
 * Contact: https://www.qt.io/licensing/
 *
 * Licensed under the GNU General Public license version 3 or any later version
 * approved by the KDE Free Qt Foundation.
 */
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
template<typename T>
constexpr typename std::add_const<T>::type& qAsConst(T& t) noexcept
{
    return t;
}

template<typename T>
void qAsConst(const T&&) = delete;

template<typename... Args>
struct QNonConstOverload
{
    template<typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }

    template<typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
struct QConstOverload
{
    template<typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
    {
        return ptr;
    }

    template<typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
struct QOverload: QConstOverload<Args...>, QNonConstOverload<Args...>
{
    using QConstOverload<Args...>::of;
    using QConstOverload<Args...>::operator();
    using QNonConstOverload<Args...>::of;
    using QNonConstOverload<Args...>::operator();

    template<typename R>
    constexpr auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }

    template<typename R>
    static constexpr auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
constexpr Q_DECL_UNUSED QOverload<Args...> qOverload = {};
template<typename... Args>
constexpr Q_DECL_UNUSED QConstOverload<Args...> qConstOverload = {};
template<typename... Args>
constexpr Q_DECL_UNUSED QNonConstOverload<Args...> qNonConstOverload = {};
#endif
/* End of code copyrighted by The Qt Company Ltd and Intel Corporation. */

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
