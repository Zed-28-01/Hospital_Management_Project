#include <gtest/gtest.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "model/Appointment.h"
#include "common/Types.h"

using namespace HMS::Model;
using namespace HMS;

// Helper: produce date and time strings offset by given minutes from now
static std::pair<std::string, std::string> makeDateTimeOffset(int minutesOffset)
{
    using namespace std::chrono;
    auto tp = system_clock::now() + minutes(minutesOffset);
    std::time_t tt = system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream date_ss, time_ss;
    date_ss << std::put_time(&tm, "%Y-%m-%d");
    time_ss << std::put_time(&tm, "%H:%M");
    return {date_ss.str(), time_ss.str()};
}

TEST(AppointmentModel, SerializeDeserializeRoundtrip)
{
    Appointment a("A001", "patientX", "D42", "2025-12-20", "09:00", "flu", 99.5, true, AppointmentStatus::COMPLETED, "ok");
    auto s = a.serialize();
    auto res = Appointment::deserialize(s);
    ASSERT_TRUE(res.has_value());
    auto b = *res;
    EXPECT_EQ(b.getAppointmentID(), "A001");
    EXPECT_EQ(b.getPatientUsername(), "patientX");
    EXPECT_EQ(b.getDoctorID(), "D42");
    EXPECT_EQ(b.getDate(), "2025-12-20");
    EXPECT_EQ(b.getTime(), "09:00");
    EXPECT_EQ(b.getDisease(), "flu");
    EXPECT_DOUBLE_EQ(b.getPrice(), 99.5);
    EXPECT_TRUE(b.isPaid());
    EXPECT_EQ(b.getStatus(), AppointmentStatus::COMPLETED);
    EXPECT_EQ(b.getNotes(), "ok");
}

TEST(AppointmentModel, DeserializeMalformed)
{
    auto res = Appointment::deserialize("too|short|fields");
    EXPECT_FALSE(res.has_value());
    auto res2 = Appointment::deserialize("A|p|d|2025-12-12|12:00|disease|not_a_price|0|0|notes");
    EXPECT_FALSE(res2.has_value());
}

TEST(AppointmentModel, StatusStringFormattingAndMutators)
{
    Appointment a("A002", "p", "d", "2025-12-20", "10:00", "x", 10.0);
    a.setStatus(AppointmentStatus::SCHEDULED);
    EXPECT_EQ(a.getStatusString(), "Scheduled");
    a.markAsCompleted();
    EXPECT_EQ(a.getStatus(), AppointmentStatus::COMPLETED);
    EXPECT_EQ(a.getStatusString(), "Completed");
    a.markAsCancelled();
    EXPECT_EQ(a.getStatus(), AppointmentStatus::CANCELLED);
    EXPECT_EQ(a.getStatusString(), "Cancelled");
    a.markAsNoShow();
    EXPECT_EQ(a.getStatus(), AppointmentStatus::NO_SHOW);
    EXPECT_EQ(a.getStatusString(), "No show");
}

TEST(AppointmentModel, UpcomingAndCancelLogic)
{
    // future appointment 2 hours from now => upcoming and canCancel
    auto dt = makeDateTimeOffset(120);
    Appointment futureA("A003", "p", "d", dt.first, dt.second, "x", 0.0);
    EXPECT_TRUE(futureA.isUpcoming());
    EXPECT_TRUE(futureA.canCancel());

    // appointment 30 minutes from now => upcoming but cannot cancel (less than 60 min)
    auto dt2 = makeDateTimeOffset(30);
    Appointment soonA("A004", "p", "d", dt2.first, dt2.second, "x", 0.0);
    EXPECT_TRUE(soonA.isUpcoming());
    EXPECT_FALSE(soonA.canCancel());

    // past appointment => not upcoming and cannot cancel
    auto dt3 = makeDateTimeOffset(-120);
    Appointment pastA("A005", "p", "d", dt3.first, dt3.second, "x", 0.0);
    EXPECT_FALSE(pastA.isUpcoming());
    EXPECT_FALSE(pastA.canCancel());
}

TEST(AppointmentModel, CanEditLogic)
{
    Appointment a("A006", "p", "d", "2025-12-20", "09:00", "x", 0.0);
    a.setStatus(AppointmentStatus::SCHEDULED);
    EXPECT_TRUE(a.canEdit());
    a.setStatus(AppointmentStatus::COMPLETED);
    EXPECT_FALSE(a.canEdit());
}
