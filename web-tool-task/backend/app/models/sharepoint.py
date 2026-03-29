from sqlalchemy import Column, String, DateTime, JSON, Integer
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.core.database import Base
import uuid

class SharePointDetails(Base):
    __tablename__ = "sharepoint_details"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    folder_name = Column(String(255), nullable=False)
    folder_path = Column(String(500))
    folder_url = Column(String(500))
    site_url = Column(String(500))
    template_name = Column(String(255))
    copied_file_url = Column(String(500))
    raw_response = Column(JSON, default=dict)
    created_at = Column(DateTime(timezone=True), server_default=func.now())